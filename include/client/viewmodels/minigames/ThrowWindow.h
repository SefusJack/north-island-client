#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_THROW_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_THROW_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/subsystems/minigames/ThrowGame.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief The ball-throwing minigame's screen: both sides with hearts, balls in the air, landing/hit effects, the strength bar and the Exit button over a ThrowGame.
    /// @details Original: ThrowWindow.xml, DrawThrowGame. Shows the Exit button the game's explain form brings.
    class ThrowWindow {
        public:
            /// @brief Loads the form and wires the painter, mouse and Exit handlers.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed on to each ThrowGame.
            /// @param playerLook Gives the player's current look.
            ThrowWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<const CharacterLook&()> playerLook
            );

            /// @brief Tells whether the game form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives the current game; only valid after open().
            /// @return The running (or last) ThrowGame.
            ThrowGame& game() { return *m_game; }

            /// @brief Starts a new game and shows the form.
            /// @details Original: RecvSportStart: CH_TThrowGame.Create and StartThrowGame (the NPC kind that fills both sides).
            /// @param npcId NPC kind filling both sides.
            /// @param nowMs Current time in milliseconds.
            void open(int npcId, std::uint64_t nowMs);

            /// @brief Starts the play after the instructions; the game's explain form with its Exit button comes.
            /// @details Original: StartFormButtonClick 1: FStateB.V[4]; the explain form shows with the first update.
            void begin();

            /// @brief Hides the form; the game object is kept until the next start.
            void close();

        private:
            /// @brief Updates the game and draws the whole screen.
            /// @details Original: UpdateThrowGame, DrawThrowGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the player's figure or a fighter in its action, with shadow and hearts.
            /// @param canvas Canvas to draw on.
            /// @param side 1 for the own team, 2 for the rival.
            /// @param index Place within the team.
            /// @param amount How far between the last frame's places and the current ones (1 = where they are).
            void drawThrower(gfx::Canvas& canvas, int side, int index, double amount);

            /// @brief Draws a heart for each life left over a person.
            /// @details Original: DrawThrowerHearts.
            /// @param canvas Canvas to draw on.
            /// @param side 1 for the own team, 2 for the rival.
            /// @param index Place within the team.
            /// @param at Screen position of the person.
            void drawHearts(gfx::Canvas& canvas, int side, int index, core::Point at);

            /// @brief Draws the strength charged so far beside the player, in its frame.
            /// @details Original: DrawThrowPowerBar.
            /// @param canvas Canvas to draw on.
            /// @param amount How far between the last frame's places and the current ones (1 = where they are).
            void drawPowerBar(gfx::Canvas& canvas, double amount);

            /// @brief Builds the hooks a ThrowGame needs from the window's services.
            /// @return The hooks.
            ThrowGame::Hooks makeHooks();

            GameServices& m_services;
            GameHooks m_hooks;
            std::function<const CharacterLook&()> m_playerLook;
            std::unique_ptr<ThrowGame> m_game;
            ui::Element& m_form;
            ui::ImageButton& m_exit;
    };

}

#endif
