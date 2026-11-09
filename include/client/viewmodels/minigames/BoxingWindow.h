#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_BOXING_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_BOXING_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/BoxingGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Golden Glove's screen: ring, strength bar, monster, boxer, score board, sliding prize and the Quit button, over a BoxingGame.
    /// @details Original: BoxingWindow.xml, DrawBoxingGame.
    class BoxingWindow {
        public:
            /// @brief Loads the form layout and wires its events.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to the BoxingGame.
            /// @param instructionsOpen Tells whether the instructions are showing over the ring.
            BoxingWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the fight state.
            /// @return The BoxingGame behind the window.
            BoxingGame& game() { return m_game; }

            /// @brief Starts the game and shows the ring under the instructions.
            /// @details Original: TSportBoxing.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @details Original: UpdateBoxingGame and DrawBoxingGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the three punches' places and what is left of 800.
            /// @details Original: DrawBoxingScoreBoard.
            /// @param canvas Canvas to draw on.
            void drawScoreBoard(gfx::Canvas& canvas) const;

            /// @brief Draws a number as three digits and the closing picture.
            /// @details Original: DrawBoxingNumber.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the number.
            /// @param value Number to draw (-1 draws dashes).
            void drawNumber(gfx::Canvas& canvas, core::Point at, int value) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            std::function<bool()> m_instructionsOpen;
            BoxingGame m_game;
            ui::Element& m_form;
            ui::ImageButton& m_quit;
            const ui::Spot& m_backgroundSpot;
            const ui::Spot& m_barFrameSpot;
            const ui::Spot& m_barSpot;
            const ui::Spot& m_monsterSpot;
            const ui::Spot& m_boxerSpot;
            const ui::Spot& m_placeSpot;
            const ui::Spot& m_leftSpot;
            const ui::Spot& m_digitSpot;
            core::Point m_lastMouse;
    };

}

#endif
