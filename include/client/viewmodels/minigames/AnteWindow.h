#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_ANTE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_ANTE_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The chips form of Five in a Row: five bets (none, 1000, 5000, 10000, 100000), the chosen one and the gold carried.
    /// @details Original: AnteWindow.xml, LD_MiniGame.pas TAnteForm. A bet is refused when the gold does not cover it or would come to more than 9999999 with it.
    class AnteWindow {
        public:
            /// @brief Callbacks the window uses to reach the game.
            struct Hooks {
                std::function<std::int64_t()> gold;
                std::function<void(int)> error;
                std::function<void(int)> confirm;
            };

            /// @brief Loads the form layout and wires its buttons.
            /// @param services Game services (layouts).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks to the game (gold, error, confirm).
            AnteWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form in front with nothing chosen yet.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

            /// @brief Shows a bet in the field, right-aligned in six places.
            /// @details Original: SetEditorText '     0'.
            /// @param gold Bet to show.
            void setBet(int gold);

            /// @brief Reads the chosen bet.
            /// @return The bet shown (0 when nothing is chosen).
            int bet() const;

        private:
            /// @brief Takes a bet when the gold covers it and the total stays under the most.
            /// @details Original: SetAnte0 .. SetAnte100000.
            /// @param gold Bet chosen.
            void chooseBet(int gold);

            /// @brief Draws the gold carried.
            /// @details Original: DrawControl.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            ui::Screen& m_screen;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_bet;
            const ui::Spot& m_goldSpot;
    };

}

#endif
