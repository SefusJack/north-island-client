#include "client/viewmodels/minigames/JigsawWindow.h"

#include <string>

namespace client {

    namespace {

        // top-left of each prize item icon (index 0 unused, prizes 1..5 left column, 6..10 right column), px
        constexpr std::array<core::Point, 11> kPrizeAt{
            {{0, 0}, {287, 361}, {287, 400}, {287, 439}, {287, 478}, {287, 518}, {610, 361}, {610, 400}, {610, 438}, {610, 478}, {610, 518}}
        };
        constexpr int kFlashPieceHeight = 70; // height of one flashing row piece of Mario_Bingo, px
        constexpr int kFlashLowerShift = 140; // y distance in Mario_Bingo from the middle row piece to the lower one, px
        constexpr std::size_t kNameChars = 14; // item names are cut to 14 characters
        const char* const kPrizeNames[] = {
            "JigsawPrize1",
            "JigsawPrize2",
            "JigsawPrize3",
            "JigsawPrize4",
            "JigsawPrize5",
            "JigsawPrize6",
            "JigsawPrize7",
            "JigsawPrize8",
            "JigsawPrize9",
            "JigsawPrize10"
        };

        /// @brief Loads JigsawWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "JigsawForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("JigsawWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("JigsawForm");
        }

        /// @brief Looks up the ten prize drawing areas of the layout.
        /// @param screen Screen holding the loaded layout.
        /// @return The areas "JigsawPrize1" to "JigsawPrize10", in order.
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

    JigsawWindow::JigsawWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_form(loadForm(services, screen)),
          m_prizes(prizeAreas(screen)),
          m_start(screen.get<ui::ImageButton>("JigsawStart")),
          m_quit(screen.get<ui::ImageButton>("JigsawQuit")),
          m_groundSpot(screen.get<ui::Spot>("JigsawGround")),
          m_stripSpot(screen.get<ui::Spot>("JigsawStrip")),
          m_bingoSpot(screen.get<ui::Spot>("JigsawBingo")),
          m_nameSpot(screen.get<ui::Spot>("JigsawName")) {
        screen.get<ui::DrawingArea>("JigsawPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onJigsawStart", [this](const ui::EventArgs&) { m_game->startClicked(); });
        screen.on("onJigsawQuit", [this](const ui::EventArgs&) { m_game->quitClicked(); });
    }

    gfx::PictureId JigsawWindow::iconOf(int itemId) const {
        const auto record = m_services.items.find(itemId);
        return record ? m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{}) : gfx::PictureId{};
    }

    void JigsawWindow::open(int kind, std::uint64_t nowMs) {
        m_game = std::make_unique<JigsawGame>(kind, m_hooks);
        m_game->open(nowMs);
        setUpPrizes();
        m_start.setVisible(false);
        m_quit.setVisible(false);
        m_form.setVisible(true);
    }

    void JigsawWindow::setUpPrizes() {
        const MachineTables::Prizes& items = m_game->prizes();
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            ui::DrawingArea& area = m_prizes[static_cast<std::size_t>(prize - 1)];
            const auto record = m_services.items.find(items[static_cast<std::size_t>(prize)]);
            const gfx::PictureId icon = iconOf(items[static_cast<std::size_t>(prize)]);
            const core::Point at = kPrizeAt[static_cast<std::size_t>(prize)];
            const core::Size size = icon.isValid() ? core::Size{m_services.pictures.picture(icon).width(), m_services.pictures.picture(icon).height()}
                                                   : core::Size{32, 32}; // fallback icon size in pixels when the item has no picture
            area.setBounds({at.x, at.y - 1, size.width, size.height}); // icons are drawn one pixel higher
            std::string hint;
            if (record) {
                // machine kind 12 shows the prize as an embeddable gem: ability points are twice the item rank
                hint = m_game->kind() == 12
                           ? record->name + ": Can be embed in equipment to improve its attribute.   " + std::to_string(record->rank * 2) + " Points ability"
                           : record->description;
            }
            area.setHint(hint);
        }
    }

    void JigsawWindow::close() {
        m_form.setVisible(false);
        m_game.reset();
    }

    void JigsawWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_game->update(nowMs, m_instructionsOpen());
        m_start.setVisible(m_game->formShown());
        m_quit.setVisible(m_game->formShown());
        gfx::PictureLibrary& pictures = m_services.pictures;
        canvas.fillRect(canvas.window(), {0, 0, 0, 255}); // opaque black (r, g, b, a)
        if (const auto ground = pictures.find("MarioGround")) {
            canvas.drawPicture(*ground, m_groundSpot.at());
        }
        const MachineTables::Prizes& items = m_game->prizes();
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            if (const gfx::PictureId icon = iconOf(items[static_cast<std::size_t>(prize)]); icon.isValid()) {
                canvas.drawPicture(
                    icon,
                    {kPrizeAt[static_cast<std::size_t>(prize)].x, kPrizeAt[static_cast<std::size_t>(prize)].y - 1}
                ); // icons are drawn one pixel higher
            }
        }
        const char* const strips[] = {"Mario_Up", "Mario_Mid", "Mario_Buttom"};
        for (int strip = 1; strip <= JigsawGame::kStrips; ++strip) {
            if (const auto picture = pictures.find(strips[strip - 1])) {
                canvas.drawPicture(
                    *picture,
                    {m_stripSpot.at().x, m_stripSpot.at().y + (strip - 1) * m_stripSpot.rect().height},
                    core::Rect{0, m_game->shownFrame(strip) * m_stripSpot.rect().height, m_stripSpot.rect().width, m_stripSpot.rect().height}
                );
            }
        }
        if (const auto bingo = pictures.find("Mario_Bingo")) {
            if (m_game->bingoFrame() >= 0) {
                canvas.drawPicture(
                    *bingo,
                    m_bingoSpot.at(),
                    core::Rect{0, m_game->bingoFrame() * m_bingoSpot.rect().width, m_bingoSpot.rect().width, m_bingoSpot.rect().width}
                );
            }
            if (m_game->flashRows() >= 0) {
                const int base = m_game->flashRows();
                canvas.drawPicture(
                    *bingo,
                    {m_bingoSpot.at().x, m_bingoSpot.at().y + m_stripSpot.rect().height},
                    core::Rect{0, base, m_bingoSpot.rect().width, kFlashPieceHeight}
                );
                canvas.drawPicture(
                    *bingo,
                    {m_bingoSpot.at().x, m_bingoSpot.at().y + 2 * m_stripSpot.rect().height},
                    core::Rect{0, base + kFlashLowerShift, m_bingoSpot.rect().width, kFlashPieceHeight}
                );
            }
        }
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            if (const auto record = m_services.items.find(items[static_cast<std::size_t>(prize)])) {
                const core::Point place = kPrizeAt[static_cast<std::size_t>(prize)];
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

}
