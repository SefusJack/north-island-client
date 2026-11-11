#include "client/viewmodels/minigames/FortuneEggWindow.h"

#include <string>

namespace client {

    namespace {

        constexpr int kNameChars = 14; // item names are cut to 14 characters
        const char* const kPrizeNames[] = {
            "FortuneEggPrize1",
            "FortuneEggPrize2",
            "FortuneEggPrize3",
            "FortuneEggPrize4",
            "FortuneEggPrize5",
            "FortuneEggPrize6",
            "FortuneEggPrize7",
            "FortuneEggPrize8",
            "FortuneEggPrize9",
            "FortuneEggPrize10"
        };

        /// @brief Loads FortuneEggWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "FortuneEggForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("FortuneEggWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("FortuneEggForm");
        }

        /// @brief Looks up the ten prize drawing areas of the layout.
        /// @param screen Screen holding the loaded layout.
        /// @return The areas "FortuneEggPrize1" to "FortuneEggPrize10", in order.
        std::array<std::reference_wrapper<ui::DrawingArea>, MachineTables::kPrizes> prizeAreas(ui::Screen& screen) {
            return {
                screen.get<ui::DrawingArea>(kPrizeNames[0]),
                screen.get<ui::DrawingArea>(kPrizeNames[1]),
                screen.get<ui::DrawingArea>(kPrizeNames[2]),
                screen.get<ui::DrawingArea>(kPrizeNames[3]),
                screen.get<ui::DrawingArea>(kPrizeNames[4]),
                screen.get<ui::DrawingArea>(kPrizeNames[5]),
                screen.get<ui::DrawingArea>(kPrizeNames[6]),
                screen.get<ui::DrawingArea>(kPrizeNames[7]),
                screen.get<ui::DrawingArea>(kPrizeNames[8]),
                screen.get<ui::DrawingArea>(kPrizeNames[9])
            };
        }

    }

    FortuneEggWindow::FortuneEggWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks
    )
        : m_services(services),
          m_egg(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_prizes(prizeAreas(screen)),
          m_play(screen.get<ui::ImageButton>("FortuneEggPlay")),
          m_exit(screen.get<ui::ImageButton>("FortuneEggExit")),
          m_backgroundSpot(screen.get<ui::Spot>("FortuneEggBackground")),
          m_buttonSpot(screen.get<ui::Spot>("FortuneEggButtonAt")),
          m_eggSpot(screen.get<ui::Spot>("FortuneEggEgg")),
          m_iconSpot(screen.get<ui::Spot>("FortuneEggIcon")),
          m_nameSpot(screen.get<ui::Spot>("FortuneEggName")) {
        screen.get<ui::DrawingArea>("FortuneEggPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onFortuneEggPlay", [this](const ui::EventArgs&) { m_egg.playClicked(); });
        screen.on("onFortuneEggExit", [this](const ui::EventArgs&) { m_egg.exitClicked(); });
    }

    gfx::PictureId FortuneEggWindow::iconOf(int itemId) const {
        const auto record = m_services.items.find(itemId);
        return record ? m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{}) : gfx::PictureId{};
    }

    void FortuneEggWindow::open(std::uint64_t nowMs) {
        m_egg.open(nowMs);
        setUpPrizes();
        m_play.setVisible(false);
        m_exit.setVisible(false);
        m_form.setVisible(true);
    }

    void FortuneEggWindow::setUpPrizes() {
        const MachineTables::Prizes& items = MachineTables::eggPrizes();
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            ui::DrawingArea& area = m_prizes[static_cast<std::size_t>(prize - 1)];
            const auto record = m_services.items.find(items[static_cast<std::size_t>(prize)]);
            const gfx::PictureId icon = iconOf(items[static_cast<std::size_t>(prize)]);
            const core::Point at = MachineTables::eggPrizeAt(prize);
            const core::Size size = icon.isValid() ? core::Size{m_services.pictures.picture(icon).width(), m_services.pictures.picture(icon).height()}
                                                   : core::Size{32, 32}; // fallback icon size in pixels when the item has no picture
            area.setBounds({at.x + m_iconSpot.at().x, at.y + m_iconSpot.at().y, size.width, size.height});
            area.setHint(record ? record->description : std::string());
        }
    }

    void FortuneEggWindow::begin() { m_egg.begin(); }

    void FortuneEggWindow::close() { m_form.setVisible(false); }

    void FortuneEggWindow::update(std::uint64_t nowMs) {
        if (!isOpen()) {
            return;
        }
        m_egg.startMusicWhenActive();
        m_egg.step(nowMs);
        const bool active = m_egg.active();
        m_play.setVisible(active && !m_egg.buttonAnimating());
        m_exit.setVisible(active && m_egg.exitShown());
    }

    void FortuneEggWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        (void)at;
        (void)nowMs;
        gfx::PictureLibrary& pictures = m_services.pictures;
        canvas.fillRect(canvas.window(), core::Color{0, 0, 0, 255}); // opaque black (r, g, b, a)
        if (const auto background = pictures.find("TrunEggBG")) {
            canvas.drawPicture(*background, m_backgroundSpot.at());
        }
        const MachineTables::Prizes& items = MachineTables::eggPrizes();
        const auto box = pictures.find("TrunEggItemDB");
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            const core::Point place = MachineTables::eggPrizeAt(prize);
            if (box) {
                canvas.drawPicture(*box, place);
            }
            if (const gfx::PictureId icon = iconOf(items[static_cast<std::size_t>(prize)]); icon.isValid()) {
                canvas.drawPicture(icon, {place.x + m_iconSpot.at().x, place.y + m_iconSpot.at().y});
            }
        }
        if (m_egg.buttonAnimating() && m_egg.buttonFrame() != FortuneEgg::kButtonFrames) {
            if (const auto button = pictures.find("TrunEggButton")) {
                canvas.drawPicture(
                    *button,
                    m_buttonSpot.at(),
                    core::Rect{0, m_egg.buttonFrame() * FortuneEgg::kButtonSize, FortuneEgg::kButtonSize, FortuneEgg::kButtonSize}
                );
            }
        }
        if (m_egg.eggAnimating() && m_egg.eggFrame() != FortuneEgg::kEggFrames) {
            if (const auto egg = pictures.find("TrunEggA")) {
                canvas
                    .drawPicture(*egg, m_eggSpot.at(), core::Rect{0, m_egg.eggFrame() * FortuneEgg::kEggHeight, FortuneEgg::kEggWidth, FortuneEgg::kEggHeight});
            }
        }
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            const auto record = m_services.items.find(items[static_cast<std::size_t>(prize)]);
            if (!record) {
                continue;
            }
            const core::Point place = MachineTables::eggPrizeAt(prize);
            canvas.drawText(
                {place.x + m_nameSpot.at().x, place.y + m_nameSpot.at().y},
                record->name.substr(0, kNameChars),
                {core::Color::fromRgb565(65535), gfx::TextEffect::Shadow, std::nullopt},
                m_nameSpot.rect().width,
                m_nameSpot.rect().height
            ); // 65535 = white in RGB565
        }
    }

}
