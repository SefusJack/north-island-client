#include "client/viewmodels/minigames/SlotMachineWindow.h"

#include <string>

namespace client {

    namespace {

        const char* const kPrizeNames[] = {
            "SlotMachinePrize1",
            "SlotMachinePrize2",
            "SlotMachinePrize3",
            "SlotMachinePrize4",
            "SlotMachinePrize5",
            "SlotMachinePrize6",
            "SlotMachinePrize7",
            "SlotMachinePrize8",
            "SlotMachinePrize9",
            "SlotMachinePrize10"
        };

        /// @brief Loads SlotMachineWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "SlotMachineForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SlotMachineWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("SlotMachineForm");
        }

        /// @brief Collects the ten prize drawing areas of the form in order.
        /// @param screen Screen holding the "SlotMachinePrize1".."SlotMachinePrize10" areas.
        /// @return References to the prize areas.
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

    SlotMachineWindow::SlotMachineWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_prizes(prizeAreas(screen)),
          m_play(screen.get<ui::ImageButton>("SlotMachinePlay")),
          m_luckyStar(screen.get<ui::ImageButton>("SlotMachineLuckyStar")),
          m_exit(screen.get<ui::ImageButton>("SlotMachineExit")),
          m_backgroundSpot(screen.get<ui::Spot>("SlotMachineBackground")),
          m_bannerSpot(screen.get<ui::Spot>("SlotMachineBanner")),
          m_flashSpot(screen.get<ui::Spot>("SlotMachineFlash")),
          m_exitSpot(screen.get<ui::Spot>("SlotMachineExitAt")),
          m_starIconSpot(screen.get<ui::Spot>("SlotMachineStarIcon")) {
        screen.get<ui::DrawingArea>("SlotMachinePage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onSlotMachinePlay", [this](const ui::EventArgs&) { m_machine->playClicked(m_nowMs); });
        screen.on("onSlotMachineLuckyStar", [this](const ui::EventArgs&) { m_machine->luckyStarClicked(); });
        screen.on("onSlotMachineExit", [this](const ui::EventArgs&) { m_machine->exitClicked(); });
    }

    gfx::PictureId SlotMachineWindow::iconOf(int itemId) const {
        const auto record = m_services.items.find(itemId);
        return record ? m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{}) : gfx::PictureId{};
    }

    void SlotMachineWindow::open(int kind, std::uint64_t nowMs) {
        m_machine = std::make_unique<SlotMachine>(kind, m_hooks);
        m_machine->open(nowMs);
        setUpPrizes();
        m_play.setVisible(false);
        m_luckyStar.setVisible(false);
        m_exit.setVisible(false);
        m_form.setVisible(true);
    }

    void SlotMachineWindow::setUpPrizes() {
        const int kind = m_machine->kind();
        const MachineTables::Prizes& items = MachineTables::slotPrizes(kind);
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            ui::DrawingArea& area = m_prizes[static_cast<std::size_t>(prize - 1)];
            const auto record = m_services.items.find(items[static_cast<std::size_t>(prize)]);
            const gfx::PictureId icon = iconOf(items[static_cast<std::size_t>(prize)]);
            const core::Point at = MachineTables::slotPrizeAt(prize);
            const core::Size size = icon.isValid() ? core::Size{m_services.pictures.picture(icon).width(), m_services.pictures.picture(icon).height()}
                                                   : core::Size{32, 32}; // default prize area size when the icon is missing
            area.setBounds({at.x, at.y, size.width, size.height});
            std::string hint;
            if (record) {
                if (kind == 8) { // machine kind 8: prizes are embeddable gems (hint shows the points ability)
                    hint = record->name + ": Can be embed in equipment to improve its attribute.   " + std::to_string(record->rank * 2) +
                           " Points ability"; // points ability is twice the gem's rank
                } else if (kind == 10) { // machine kind 10: the hint is just the item's name
                    hint = record->name;
                } else {
                    hint = record->description;
                }
            }
            area.setHint(hint);
        }
        m_exit.setPosition(m_exitSpot.at());
    }

    void SlotMachineWindow::begin() { m_machine->begin(); }

    void SlotMachineWindow::close() {
        m_form.setVisible(false);
        m_machine.reset();
    }

    void SlotMachineWindow::update(std::uint64_t nowMs) {
        if (!isOpen() || !m_machine) {
            return;
        }
        m_nowMs = nowMs;
        m_machine->startMusicWhenActive();
        m_machine->step(nowMs);
        const bool active = m_machine->active();
        m_play.setVisible(active);
        m_exit.setVisible(active && m_machine->exitShown());
        m_luckyStar.setVisible(active && m_machine->luckyStarShown());
        m_luckyStar.setAnimating(m_machine->luckyStarOn());
    }

    void SlotMachineWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        (void)at;
        if (!m_machine) {
            return;
        }
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto background = pictures.find("SlotmachBG")) {
            canvas.drawPicture(*background, m_backgroundSpot.at());
        }
        const MachineTables::Prizes& items = MachineTables::slotPrizes(m_machine->kind());
        for (int prize = 1; prize <= MachineTables::kPrizes; ++prize) {
            if (const gfx::PictureId icon = iconOf(items[static_cast<std::size_t>(prize)]); icon.isValid()) {
                canvas.drawPicture(icon, MachineTables::slotPrizeAt(prize));
            }
        }
        if (const auto ring = pictures.find("SlotmachRing")) {
            for (int reel = 1; reel <= SlotMachine::kReels; ++reel) {
                const int top = (m_machine->reelPosition(reel) - 1) * SlotMachine::kReelHeight;
                canvas.drawPicture(*ring, MachineTables::reelAt(reel), core::Rect{0, top, SlotMachine::kReelWidth, SlotMachine::kReelHeight});
            }
        }
        if (m_machine->prizeFlash()) {
            if (const auto flash = pictures.find("SlotmachItemT")) {
                const core::Point place = MachineTables::slotPrizeAt(m_machine->prizeSlot());
                canvas.drawPicture(
                    *flash,
                    {place.x + m_flashSpot.at().x, place.y + m_flashSpot.at().y},
                    core::Rect{0, (m_machine->flashFrame() - 1) * SlotMachine::kFlashHeight, SlotMachine::kFlashWidth, SlotMachine::kFlashHeight}
                );
            }
        }
        if (m_luckyStar.isVisible()) {
            if (const auto star = pictures.find(m_machine->luckyStarOn() ? "Icon_LSUse" : "Icon_LSNoUse")) {
                const core::Rect button = m_luckyStar.bounds();
                canvas.drawPicture(*star, {button.x, button.y + button.height + m_starIconSpot.at().y});
            }
        }
        if (const auto banner = pictures.find("SlotmachText")) {
            const gfx::Picture& picture = pictures.picture(*banner);
            const int slide = m_machine->bannerSlide(nowMs);
            if (slide == 0) {
                canvas.drawPicture(*banner, m_bannerSpot.at());
            } else if (slide < picture.height()) {
                canvas.drawPicture(*banner, {m_bannerSpot.at().x, m_bannerSpot.at().y + slide}, core::Rect{0, 0, picture.width(), picture.height() - slide});
            }
        }
    }

}
