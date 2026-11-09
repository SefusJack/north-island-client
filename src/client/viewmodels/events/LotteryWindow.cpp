#include "client/viewmodels/events/LotteryWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/LotteryRequests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kDrawsADay = 3; // draws allowed per day
        constexpr int kFirstPrizeSpot = 20; // number of prize places; the fast spin counts down from here
        constexpr std::uint64_t kStepMs = 80; // time per prize place during the fast spin, ms
        constexpr std::uint64_t kFastMs = 1600; // end of the fast spin, ms after the draw started
        constexpr std::uint64_t kEndMs = 2800; // end of the whole animation (draw request is sent), ms

        /// @brief Loads LotteryWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "LotteryWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LotteryWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("LotteryWindow");
        }

    }

    LotteryWindow::LotteryWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        std::function<void(const core::Bytes&)> send
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_messages(messages),
          m_send(std::move(send)),
          m_window(loadWindow(services, screen)),
          m_prizeArea(screen.get<ui::DrawingArea>("LotteryPrize")),
          m_drawsLeftSpot(screen.get<ui::Spot>("LotteryDrawsLeft")),
          m_textColorSpot(screen.get<ui::Spot>("LotteryTextColor")),
          m_prizesWindow(services, screen, m_prizes, info),
          m_unknownIcon(services.pictures.find("Icon_UnKnow").value_or(gfx::PictureId{})) {
        screen.get<ui::DrawingArea>("LotteryPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        m_prizeArea.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintPrize(canvas, at); });
        screen.on("onLotteryClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onLotteryDraw", [this](const ui::EventArgs&) { draw(); });
        screen.on("onLotteryPrize", [this](const ui::EventArgs&) { m_prizesWindow.open(); });
    }

    void LotteryWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_prizeShown = false;
        m_shownIcon = {};
        m_drawing = false;
        m_drawStartMs = 0;
        m_phase = 0;
        m_shown = 0;
    }

    void LotteryWindow::infoReceived(const protocol::LotteryInfo& info) {
        m_drawsUsed = info.drawsUsed;
        m_prizes = info.prizes;
        m_prizesWindow.refresh();
    }

    void LotteryWindow::resultReceived(const protocol::LotteryResult& result) {
        m_drawsUsed = result.drawsUsed;
        if (result.prize <= kFirstPrizeSpot) { // at most 20 prize places
            showPrize(result.prize);
        }
    }

    bool LotteryWindow::hasFreeCell() const {
        for (int cell = 1; cell <= Inventory::kBagSlots; ++cell) {
            if (m_inventory.occupantOf(cell) == 0) {
                return true;
            }
        }
        return false;
    }

    void LotteryWindow::draw() {
        if (m_drawing) {
            return;
        }
        if (kDrawsADay - m_drawsUsed <= 0) {
            m_messages.show("No draws left for today. Please log-in again tomorrow to draw more", 1000); // message time in ms
        } else if (!hasFreeCell()) {
            m_messages.show("Not enough space", 1000); // message time in ms
        } else {
            m_drawing = true;
            m_drawStartMs = m_nowMs;
        }
    }

    void LotteryWindow::showPrize(int number) {
        const int itemId = number >= 1 && number <= static_cast<int>(m_prizes.size()) ? m_prizes[static_cast<std::size_t>(number - 1)].itemId : 0;
        const auto record = m_services.items.find(itemId);
        m_shownIcon = m_services.pictures.find(std::to_string(record ? record->iconId : 0)).value_or(gfx::PictureId{});
        m_prizeShown = true;
        m_shown = number;
    }

    void LotteryWindow::showRandomPrize() {
        const int count = static_cast<int>(m_prizes.size());
        int chosen = m_shown;
        for (int tries = 1; chosen == m_shown && tries <= 100; ++tries) { // at most 100 tries
            chosen = (count > 0 ? std::uniform_int_distribution<int>(0, count - 1)(m_random) : 0) + 1;
        }
        showPrize(chosen);
    }

    void LotteryWindow::animate(std::uint64_t nowMs) {
        const std::uint64_t passed = nowMs - m_drawStartMs;
        if (passed >= kFastMs) {
            if (passed >= kEndMs) {
                m_drawing = false;
                m_drawStartMs = 0;
                m_phase = 0;
                m_shown = 0;
                m_send(protocol::lotteryDrawRequest());
                return;
            }
            // slow-down steps at 1.6, 1.8, 2.0 and 2.3 s (ms) give the phases 99..102
            // (kFastMs = 1600 starts phase 99; phase numbers continue after the 20 fast-spin steps)
            const int phase = passed >= 2300 ? 102 : passed >= 2000 ? 101 : passed >= 1800 ? 100 : 99;
            if (m_phase >= phase) {
                return;
            }
            m_phase = phase;
            showRandomPrize();
            return;
        }
        for (int step = kFirstPrizeSpot; step >= 1; --step) {
            if (m_phase >= step) {
                break;
            }
            if (passed >= static_cast<std::uint64_t>(step) * kStepMs) {
                m_phase = step;
                showRandomPrize();
            }
        }
    }

    void LotteryWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        m_nowMs = nowMs;
        canvas.drawText(
            {at.x + m_drawsLeftSpot.at().x, at.y + m_drawsLeftSpot.at().y},
            std::to_string(kDrawsADay - m_drawsUsed),
            {m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            100
        ); // text box width
        if (m_drawing) {
            animate(nowMs);
        }
    }

    void LotteryWindow::paintPrize(gfx::Canvas& canvas, core::Point at) {
        if (m_shownIcon.isValid()) {
            canvas.drawPicture(m_shownIcon, at);
        } else if (m_prizeShown) {
            constexpr int kIconSize = 32; // item icon size, px
            // white box (RGBA 255,255,255,255) with a light blue outline (140,170,206)
            canvas.fillRect({at.x, at.y, kIconSize, kIconSize}, {255, 255, 255, 255});
            canvas.drawRectOutline({at.x, at.y, kIconSize, kIconSize}, {140, 170, 206, 255});
        } else if (m_unknownIcon.isValid()) {
            canvas.drawPicture(m_unknownIcon, at, core::Rect{3, 2, 35, 34}); // question mark: 35 by 34 from the picture's (3, 2)
        }
    }

}
