#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_SHEEP_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_SHEEP_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/SheepGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <memory>

namespace client {

    /// @brief The dream minigame's screen: backdrop, hearts, score and time, the sleeper with its dream and the hammer, hit flash and messages over a SheepGame.
    /// @details Original: SheepWindow.xml, DrawSheepDreamGame. Shows the Exit button the game's explain form brings.
    class SheepWindow {
        public:
            /// @brief Loads the form and wires the painter, click and Exit handlers.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed on to each SheepGame.
            SheepWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks
            );

            /// @brief Tells whether the game form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives the current game; only valid after open().
            /// @return The running (or last) SheepGame.
            SheepGame& game() { return *m_game; }

            /// @brief Starts a new game and shows the form.
            /// @details Original: RecvSportStart: Tsport_SheepDream.Create (gameKind from the packet's level byte).
            /// @param gameKind Game kind (1 sheep, otherwise pig variants).
            /// @param nowMs Current time in milliseconds.
            void open(int gameKind, std::uint64_t nowMs);

            /// @brief Starts the play after the instructions and switches the cursor to the hammer.
            /// @details Original: StartFormButtonClick 1: SetMouseCursorType 13 (the hammer).
            /// @param nowMs Current time in milliseconds.
            void begin(std::uint64_t nowMs);

            /// @brief Hides the form and restores the cursor; the game object is kept until the next start.
            void close();

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @details Original: UpdateSheepDreamGame, DrawSheepDreamGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the pictures behind, the hearts, labels, score, time and the end words.
            /// @details Original: DrawSheepDreamHud.
            /// @param canvas Canvas to draw on.
            void drawHud(gfx::Canvas& canvas);

            /// @brief Draws the sleeper, its dream and the hammer.
            /// @details Original: DrawSheepDreamScene.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void drawScene(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Builds the hooks a SheepGame needs from the window's services.
            /// @return The hooks.
            SheepGame::Hooks makeHooks();

            /// @brief Looks up the sleeper's NPC in the table (17371 the sheep, 17400 the pig).
            /// @return The sleeper's NPC info.
            formats::NpcInfo sleeper() const;

            GameServices& m_services;
            GameHooks m_hooks;
            std::unique_ptr<SheepGame> m_game;
            ui::Element& m_form;
            ui::DrawingArea& m_page;
            ui::ImageButton& m_exit;
            const ui::Spot& m_skySpot;
            const ui::Spot& m_hillsSpot;
            const ui::Spot& m_cloudsSpot;
            const ui::Spot& m_boardSpot;
            const ui::Spot& m_scoreSpot;
            const ui::Spot& m_lifeSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_scoreDigitsSpot;
            const ui::Spot& m_timeDigitsSpot;
            const ui::Spot& m_blinkSheepSpot;
            const ui::Spot& m_blinkPigSpot;
            const ui::Spot& m_sleeperSpot;
            const ui::Spot& m_dreamSpot;
            const ui::Spot& m_hammerSpot;
            const ui::Spot& m_effectSpot;
            const ui::Spot& m_messageSpot;
            const ui::Spot& m_heartSpot;
    };

}

#endif
