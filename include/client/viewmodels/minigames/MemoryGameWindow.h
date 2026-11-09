#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_MEMORY_GAME_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_MEMORY_GAME_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/MemoryGame.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The memory minigame's screen: board, hearts, time, score, circles and their numbers over a MemoryGame.
    /// @details Original: MemoryGameWindow.xml, DrawMemoryGame. The board shows under the instructions already.
    class MemoryGameWindow {
        public:
            /// @brief Loads the form and wires the painter, click and Leave handlers.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed to the MemoryGame.
            /// @param instructionsOpen Tells whether the instructions window is open.
            MemoryGameWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen
            );

            /// @brief Tells whether the game form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives the game state.
            /// @return The MemoryGame.
            MemoryGame& game() { return m_game; }

            /// @brief Starts the game and shows the board.
            /// @details Original: TSport_MemoryGame.Create.
            /// @param nowMs Current time in milliseconds.
            void open(std::uint64_t nowMs);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Steps the game and draws the whole screen.
            /// @details Original: DrawMemoryGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            GameServices& m_services;
            std::function<bool()> m_instructionsOpen;
            MemoryGame m_game;
            ui::Element& m_form;
            ui::ImageButton& m_leave;
            const ui::Spot& m_boardSpot;
            const ui::Spot& m_heartSpot;
            const ui::Spot& m_scoreSpot;
            const ui::Spot& m_scoreNumberSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_timeNumberSpot;
            const ui::Spot& m_messageSpot;
            const ui::Spot& m_rightSignSpot;
            const ui::Spot& m_wrongSignSpot;
            const ui::Spot& m_cardNumberSpot;
            gfx::PictureId m_heart;
            std::uint64_t m_frameMs = 0;
    };

}

#endif
