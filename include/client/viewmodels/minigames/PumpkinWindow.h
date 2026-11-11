#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_PUMPKIN_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_PUMPKIN_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/effects/LightEffect.h"
#include "client/subsystems/minigames/PumpkinGame.h"
#include "client/subsystems/minigames/SportScene.h"
#include "formats/NpcInfo.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <random>

namespace client {

    /// @brief The pumpkin minigame's screen: field, player figure with the swinging arrow, ghosts, pumpkin in flight, hearts, score, time and messages over a PumpkinGame.
    /// @details Original: PumpkinWindow.xml, DrawPumpkinGame. A sport scene of kind 17.
    class PumpkinWindow : public SportScene {
        public:
            /// @brief Loads the form and wires the painter, mouse-down and Quit handlers.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed on to each PumpkinGame.
            /// @param instructionsOpen Tells whether the instructions window is open.
            /// @param playerLook Gives the player's current look.
            PumpkinWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen,
                std::function<const CharacterLook&()> playerLook
            );

            /// @brief Sport scene kind of this game.
            /// @return 17.
            int kind() const override { return 17; } // sport scene kind id of the pumpkin game

            /// @brief Starts a new game and shows the field (under the instructions).
            /// @details Original: RecvSportStart, TPumpkin.Create.
            /// @param nowMs Current time in milliseconds.
            void open(std::uint64_t nowMs) override;

            /// @brief Hides the form and stops the hit light; the game object is kept until the next start.
            void close() override;

            /// @brief Leaves the game (same as the Quit button).
            void leave() override;

            /// @brief Begins play after the instructions.
            void begin(std::uint64_t nowMs) override;

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @details Original: UpdatePumpkinGame, DrawPumpkinGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the field, its front picture and the player's figure.
            /// @param canvas Canvas to draw on.
            void drawField(gfx::Canvas& canvas);

            /// @brief Draws the pumpkin in flight, then each walking ghost.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between the last frame's places and the current ones (1 = where they are).
            void drawGhosts(gfx::Canvas& canvas, double amount);

            /// @brief Draws hearts, score, time, life label, countdown and end words.
            /// @param canvas Canvas to draw on.
            void drawStatus(gfx::Canvas& canvas);

            /// @brief Draws the swinging aim arrow while playing.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between the last frame's places and the current ones (1 = where they are).
            void drawAimLine(gfx::Canvas& canvas, double amount);

            /// @brief Builds the hooks a PumpkinGame needs from the window's services.
            /// @return The hooks.
            PumpkinGame::Hooks makeHooks();

            GameServices& m_services;
            GameHooks m_hooks;
            std::function<bool()> m_instructionsOpen;
            std::function<const CharacterLook&()> m_playerLook;
            std::optional<formats::NpcInfo> m_ghost;
            std::unique_ptr<PumpkinGame> m_game;
            std::mt19937 m_random{std::random_device{}()};
            LightEffect m_light;
            std::uint64_t m_frameMs = 0;
            ui::Element& m_form;
            ui::ImageButton& m_quit;
            const ui::Spot& m_fieldSpot;
            const ui::Spot& m_frontSpot;
            const ui::Spot& m_heartSpot;
            const ui::Spot& m_lifeSpot;
            const ui::Spot& m_scoreSpot;
            const ui::Spot& m_scoreNumberSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_timeNumberSpot;
            const ui::Spot& m_messageSpot;
            const ui::Spot& m_aimSpot;
            const ui::Spot& m_coinSpot;
    };

}

#endif
