#include "client/viewmodels/fish/FishBuyWindow.h"

#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kWidth = 243; // window width, px
        constexpr int kEdge = 800; // screen width, px; the window must not stick out past it
        constexpr int kMargin = 20; // gap kept to the screen edge, px
        constexpr int kSmallHalf = 20; // fish whose half frame width is below this (px) count as small
        constexpr int kSmallShift = 15; // x shift of the preview for small fish, px
        constexpr int kFirstBuyable = 3; // first fish kind that can be bought (kinds 1 and 2 are the tank's permanent fish)
        constexpr int kLastBuyable = 7; // last fish kind (FishTank::kKinds)
        constexpr int kPrices[] = {0, 0, 0, 50, 60, 10, 100, 150}; // gold price per fish kind 0..7 (kinds 1-2 are not for sale)

        /// @brief Loads FishBuyWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "FishBuy" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("FishBuyWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("FishBuy");
        }

    }

    FishBuyWindow::FishBuyWindow(
        GameServices& services,
        ui::Screen& screen,
        FishTankWindow& tank
    )
        : m_services(services),
          m_screen(screen),
          m_tank(tank),
          m_window(loadWindow(services, screen)),
          m_fishSpot(screen.get<ui::Spot>("FishBuyFish")),
          m_fishOffsetSpot(screen.get<ui::Spot>("FishBuyFishOffset")),
          m_goldSpot(screen.get<ui::Spot>("FishBuyGold")),
          m_priceSpot(screen.get<ui::Spot>("FishBuyPrice")),
          m_goldIcon(services.pictures.find("GdMoney2").value_or(gfx::PictureId{})) {
        for (int kind = 0; kind <= FishTank::kKinds; ++kind) {
            TankFish& fish = m_fishes[static_cast<std::size_t>(kind)];
            fish.setPicture(tank.tank().kindPicture(kind), tank.tank().kindSize(kind), tank.tank().randomFunction());
            fish.posX = m_fishSpot.at().x;
            fish.posY = m_fishSpot.at().y;
        }
        screen.get<ui::DrawingArea>("FishBuyPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.on("onFishBuyClose", [this](const ui::EventArgs&) { hide(); });
        screen.on("onFishBuyOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onFishBuyPrevious", [this](const ui::EventArgs&) { previousKind(); });
        screen.on("onFishBuyNext", [this](const ui::EventArgs&) { nextKind(); });
    }

    void FishBuyWindow::show(int mode) {
        m_mode = mode;
        const core::Rect box = m_tank.bounds();
        int left = box.x - kWidth < 0 ? box.x + box.width : box.x - kWidth;
        if (left + kWidth >= kEdge) {
            const int shift = kWidth - (kEdge - left) + kMargin;
            left -= shift;
            m_tank.moveBy(-shift);
        }
        m_window.setPicture(mode == 1 ? "FormBuyFish" : "FormDelFish", m_services.pictures);
        m_window.setPosition({left, box.y});
        if (mode == 1) { // mode 1: buy a fish
            m_kind = kFirstBuyable;
        } else {
            m_index = 0;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void FishBuyWindow::confirm() {
        if (m_mode == 1) { // mode 1: buy a fish
            m_tank.requestKind(2, m_kind); // request code 2: buy this kind
            return;
        }
        const std::vector<int>& kinds = m_tank.tank().kinds();
        if (m_index < 0 || m_index >= static_cast<int>(kinds.size())) {
            return;
        }
        m_tank.tank().removeKind(static_cast<std::size_t>(m_index));
        m_tank.requestKind(5, m_kind); // request code 5: discard this kind
        hide();
    }

    void FishBuyWindow::previousKind() {
        if (m_mode == 1) {
            --m_kind;
            if (m_kind < kFirstBuyable || m_kind > kLastBuyable) {
                m_kind = kLastBuyable;
            }
            return;
        }
        const std::vector<int>& kinds = m_tank.tank().kinds();
        if (kinds.empty()) {
            return;
        }
        --m_index;
        if (m_index < 0) {
            m_index = static_cast<int>(kinds.size()) - 1;
        }
        m_kind = kinds[static_cast<std::size_t>(m_index)];
    }

    void FishBuyWindow::nextKind() {
        if (m_mode == 1) {
            ++m_kind;
            if (m_kind > kLastBuyable || m_kind < kFirstBuyable) {
                m_kind = kFirstBuyable;
            }
            return;
        }
        const std::vector<int>& kinds = m_tank.tank().kinds();
        if (kinds.empty()) {
            return;
        }
        ++m_index;
        if (static_cast<int>(kinds.size()) <= m_index) {
            m_index = 0;
        }
        m_kind = kinds[static_cast<std::size_t>(m_index)];
    }

    void FishBuyWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        TankFish& fish = m_fishes[static_cast<std::size_t>(m_kind)];
        const int offset = fish.frameWidth / 2 < kSmallHalf ? kSmallShift : 0; // half the frame width, compared with the small limit
        fish.draw(canvas, {at.x + m_fishOffsetSpot.at().x + offset, at.y + m_fishOffsetSpot.at().y}, m_tank.bounds().width, nowMs);
        if (m_mode != 1) {
            return;
        }
        if (m_goldIcon.isValid()) {
            canvas.drawPicture(
                m_goldIcon,
                at + m_goldSpot.at(),
                core::Rect{0, 0, m_goldSpot.rect().width, m_goldSpot.rect().height}
            ); // the gold icon from the picture's origin
        }
        const gfx::TextStyle style{m_priceSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        canvas.drawText(at + m_priceSpot.at(), std::to_string(kPrices[m_kind]), style, kWidth, 15); // 15: text box height
    }

}
