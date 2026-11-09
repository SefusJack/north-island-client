#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_SCOTD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_SCOTD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/subsystems/minigames/ScotdGame.h"
#include "client/subsystems/minigames/SportScene.h"
#include "formats/NpcInfo.h"
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

    /// @brief The Scotd minigame's screen: field, hearts, player figure and NPCs with shadows, hit flash and end words over a ScotdGame.
    /// @details Original: ScotdWindow.xml, DrawScotdGame. Shows the Exit button the game's explain form brings. A sport scene of kind 2.
    class ScotdWindow : public SportScene {
        public:
            /// @brief Loads the form and wires the painter, mouse and Exit handlers.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed on to each ScotdGame.
            /// @param playerLook Gives the player's current look.
            ScotdWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<const CharacterLook&()> playerLook
            );

            /// @brief Sport scene kind of this game.
            /// @return 2.
            int kind() const override { return 2; } // sport scene kind id of the Scotd game

            /// @brief Remembers the NPC kind and level the game will start with.
            /// @details Original: RecvSportStart: the NPC kind (the value of 57/1) and the level, which takes hearts off the five.
            /// @param npcId NPC kind.
            /// @param level Game level; hearts = 5 - level.
            void startedWith(int npcId, int level) override;

            /// @brief Creates the game and shows the form.
            /// @details Original: RecvSportStart: TSport_scotd.Create(NpcId, 5 - level).
            /// @param nowMs Current time in milliseconds.
            void open(std::uint64_t nowMs) override;

            /// @brief Hides the form; the game object is kept until the next start.
            void close() override;

            /// @brief Leaves the game (same as the Exit button).
            void leave() override;

            /// @brief Begins play after the instructions.
            void begin(std::uint64_t nowMs) override;

            /// @brief Passes a key press to the running game.
            /// @details Original: HandleSportKeyDown.
            /// @param key The key pressed.
            void keyDown(ScotdGame::Key key);

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @details Original: UpdateScotdGame, DrawScotdGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the field, its front and the hearts.
            /// @details Original: DrawScotdBackground.
            /// @param canvas Canvas to draw on.
            void drawField(gfx::Canvas& canvas);

            /// @brief Draws a role's figure (the player's or the NPC's).
            /// @param canvas Canvas to draw on.
            /// @param role The role to draw.
            /// @param player True for the player's figure, false for the NPC.
            /// @param amount How far between where the last frame drew it and where it is (1 = where it is).
            void drawFigure(gfx::Canvas& canvas, const ScotdRole& role, bool player, double amount);

            /// @brief Draws a role's shadow.
            /// @param canvas Canvas to draw on.
            /// @param role The role whose shadow is drawn.
            /// @param amount How far between where the last frame drew it and where it is (1 = where it is).
            void drawShadow(gfx::Canvas& canvas, const ScotdRole& role, double amount);

            /// @brief Maps the game's slot to the slot the NPC's sprite really has.
            /// @details Original: GetSpriteDirection.
            /// @param slot Slot the game asks for.
            /// @return The sprite's slot.
            int npcSlot(int slot);

            /// @brief Builds the hooks a ScotdGame needs from the window's services.
            /// @return The hooks.
            ScotdGame::Hooks makeHooks();

            GameServices& m_services;
            GameHooks m_hooks;
            std::function<const CharacterLook&()> m_playerLook;
            std::optional<formats::NpcInfo> m_npc;
            std::unique_ptr<ScotdGame> m_game;
            int m_npcId = 0;
            int m_level = 0;
            std::mt19937 m_random{std::random_device{}()};
            ui::Element& m_form;
            ui::ImageButton& m_exit;
            const ui::Spot& m_fieldSpot;
            const ui::Spot& m_frontSpot;
            const ui::Spot& m_heartSpot;
            const ui::Spot& m_messageSpot;
    };

}

#endif
