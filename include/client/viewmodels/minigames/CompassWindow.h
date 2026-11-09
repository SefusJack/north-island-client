#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_COMPASS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_COMPASS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/subsystems/minigames/CompassGame.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Golden Compass's overlay: the player's small face with the lives left at the bottom right and the players' names at the left, over a CompassGame.
    /// @details Original: CompassWindow.xml, TWaterMelonGame's drawing.
    class CompassWindow {
        public:
            /// @brief Loads the overlay layout and sets its painter.
            /// @param services Game services (layouts, characters).
            /// @param screen Screen the overlay is loaded into.
            /// @param hooks Callbacks handed to the CompassGame.
            /// @param ownLook Returns the look of the player's own character.
            CompassWindow(
                GameServices& services,
                ui::Screen& screen,
                CompassGame::Hooks hooks,
                std::function<const CharacterLook&()> ownLook
            );

            /// @brief Tells whether the overlay is showing.
            /// @return True when the page is visible.
            bool isOpen() const { return m_page.isVisible(); }

            /// @brief Gives access to the game state.
            /// @return The CompassGame behind the window.
            CompassGame& game() { return m_game; }

            /// @brief Shows the overlay.
            void open() { m_page.setVisible(true); }

            /// @brief Hides the overlay.
            void close() { m_page.setVisible(false); }

        private:
            /// @brief Draws the player's face and lives, the countdown and the names.
            /// @details Original: TWaterMelonGame.DrawControl.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            GameServices& m_services;
            std::function<const CharacterLook&()> m_ownLook;
            CompassGame m_game;
            ui::DrawingArea& m_page;
            const ui::Spot& m_avatarSpot;
            const ui::Spot& m_multiplySpot;
            const ui::Spot& m_livesSpot;
            const ui::Spot& m_participantsSpot;
            const ui::Spot& m_ownPlaceSpot;
    };

}

#endif
