#include "client/viewmodels/minigames/AnteWindow.h"

#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <cstdio>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kMostGold = 9999999; // most gold a player can hold

        /// @brief Loads AnteWindow.xml and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "AnteWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("AnteWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("AnteWindow");
        }

        /// @brief Formats a bet right-aligned in six places, as the game writes it.
        /// @param gold Bet to format.
        /// @return The formatted text.
        std::string betText(int gold) {
            char text[16]; // buffer size, enough for a 6-place int
            std::snprintf(text, sizeof text, "%6d", gold);
            return text;
        }

    }

    AnteWindow::AnteWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_screen(screen),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_bet(screen.get<ui::TextBox>("AnteEdit")),
          m_goldSpot(screen.get<ui::Spot>("AnteGold")) {
        screen.get<ui::DrawingArea>("AntePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onAnteCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onAnteOk", [this](const ui::EventArgs&) { m_hooks.confirm(bet()); });
        screen.on("onAnteBet", [this](const ui::EventArgs& args) { chooseBet(std::atoi(args.source.c_str() + std::string("AnteBet").size())); });
        setBet(0);
    }

    void AnteWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void AnteWindow::setBet(int gold) { m_bet.setText(betText(gold)); }

    int AnteWindow::bet() const { return std::atoi(m_bet.text().c_str()); }

    void AnteWindow::chooseBet(int gold) {
        if (gold != 0) {
            if (m_hooks.gold() < gold) {
                m_hooks.error(1); // error 1: not enough gold for the bet
                return;
            }
            if (m_hooks.gold() + gold > kMostGold) {
                m_hooks.error(2); // error 2: gold plus the bet would pass the 9,999,999 limit
                return;
            }
        }
        setBet(gold);
    }

    void AnteWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_window.isVisible()) {
            // 100 x 20: width and height of the gold text box
            canvas.drawText(at + m_goldSpot.at(), std::to_string(m_hooks.gold()), {m_goldSpot.color(), gfx::TextEffect::Plain, std::nullopt}, 100, 20);
        }
    }

}
