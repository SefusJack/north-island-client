#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_LUCKY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_LUCKY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/effects/LightEffect.h"
#include "client/subsystems/minigames/LuckyGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief The catching minigame's screen: backdrop, figures, hearts, falling coins and bombs, score, time and messages over a LuckyGame.
    /// @details Original: LuckyWindow.xml, DrawLuckeyGame.
    class LuckyWindow {
        public:
            /// @brief Loads the form and wires the painter and the Quit button.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed on to each LuckyGame.
            /// @param instructionsOpen Tells whether the instructions window is open.
            /// @param playerLook Gives the player's current look.
            LuckyWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen,
                std::function<const CharacterLook&()> playerLook
            );

            /// @brief Tells whether the game form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives the current game; only valid after open().
            /// @return The running (or last) LuckyGame.
            LuckyGame& game() { return *m_game; }

            /// @brief Starts a new game and shows the form (under the instructions).
            /// @details Original: RecvSportStart, TLuckey.Create.
            /// @param npcId NPC that throws the objects.
            /// @param nowMs Current time in milliseconds.
            void open(int npcId, std::uint64_t nowMs);

            /// @brief Hides the form and stops the bomb light; the game object is kept until the next start.
            void close();

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the player's figure (with shadow) and the thrower.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between the last frame's places and the current ones (1 = the current ones).
            void drawFigures(gfx::Canvas& canvas, double amount);

            /// @brief Draws the falling coins and bombs, then the bomb light.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            /// @param amount How far between the last frame's places and the current ones (1 = the current ones).
            void drawObjects(gfx::Canvas& canvas, std::uint64_t nowMs, double amount);

            /// @brief Draws one heart for each life left.
            /// @param canvas Canvas to draw on.
            void drawHearts(gfx::Canvas& canvas);

            /// @brief Builds the hooks a LuckyGame needs from the window's services.
            /// @return The hooks.
            LuckyGame::Hooks makeHooks();

            /// @brief Picks the slot the thrower's sprite is drawn in: the action's own, or one the sprite has.
            /// @param slot Wanted action slot.
            /// @return Slot to draw.
            int throwerSlot(int slot) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            GameHooks m_hooks;
            std::function<bool()> m_instructionsOpen;
            std::function<const CharacterLook&()> m_playerLook;
            std::unique_ptr<LuckyGame> m_game;
            LightEffect m_light;
            int m_lastDropX = 0;
            ui::Element& m_form;
            ui::ImageButton& m_quit;
            const ui::Spot& m_backgroundSpot;
            const ui::Spot& m_topSpot;
            const ui::Spot& m_bottomSpot;
            const ui::Spot& m_lifeSpot;
            const ui::Spot& m_scoreSpot;
            const ui::Spot& m_scoreNumberSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_timeNumberSpot;
            const ui::Spot& m_messageSpot;
            const ui::Spot& m_heartSpot;
    };

}

#endif
