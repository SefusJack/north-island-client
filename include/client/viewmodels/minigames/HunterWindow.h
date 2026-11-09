#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_HUNTER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_HUNTER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/HunterGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief The hunting game's screen: forest in three layers, hearts, score and time, animals with life bars, the sight and shot flash, and the words over the play, with the Exit button, over a HunterGame.
    /// @details Original: HunterWindow.xml, DrawHunterGame.
    class HunterWindow {
        public:
            /// @brief Loads the form layout and wires its events.
            /// @param services Game services (layouts, pictures, npcs, characters).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to each HunterGame.
            /// @param playerBodyType Returns the player's body type.
            HunterWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<int()> playerBodyType
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the game state (only valid while a game exists).
            /// @return The HunterGame behind the window.
            HunterGame& game() { return *m_game; }

            /// @brief Creates a fresh game and shows the forest under the instructions.
            /// @details Original: RecvSportStart, TSport_Hunter.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Hides the form; the game is kept until the next one starts.
            void close();

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @details Original: UpdateHunterGame and DrawHunterGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the forest, its trees and the front of it, shaken with the screen.
            /// @details Original: DrawHunterHud's pictures.
            /// @param canvas Canvas to draw on.
            void drawField(gfx::Canvas& canvas);

            /// @brief Draws the field, the hearts, the labels, the score and the time.
            /// @details Original: DrawHunterHud.
            /// @param canvas Canvas to draw on.
            void drawHud(gfx::Canvas& canvas);

            /// @brief Draws the life bar over each animal, the animal itself (lit under the mouse) and the picture of its attack.
            /// @details Original: DrawHunterAnimals.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between where the last frame took them from and to to draw them (1 = where they are).
            void drawAnimals(gfx::Canvas& canvas, double amount);

            /// @brief Draws the sight where the mouse is and the flash of a shot where it was fired.
            /// @details Original: DrawHunterCrosshair.
            /// @param canvas Canvas to draw on.
            void drawCrosshair(gfx::Canvas& canvas);

            /// @brief Builds the hooks the HunterGame needs (npc lookup, sprite size, frame count, hit test, body type).
            /// @return The hooks, including the shared game hooks.
            HunterGame::Hooks makeHooks();

            GameServices& m_services;
            ui::Screen& m_screen;
            GameHooks m_hooks;
            std::function<int()> m_playerBodyType;
            std::unique_ptr<HunterGame> m_game;
            ui::Element& m_form;
            ui::ImageButton& m_exit;
            const ui::Spot& m_forestSpot;
            const ui::Spot& m_frontSpot;
            const ui::Spot& m_scoreSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_lifeSpot;
            const ui::Spot& m_scoreDigitsSpot;
            const ui::Spot& m_timeDigitsSpot;
            const ui::Spot& m_attackSpot;
            const ui::Spot& m_messageSpot;
            const ui::Spot& m_heartSpot;
            core::Point m_lastMouse;
    };

}

#endif
