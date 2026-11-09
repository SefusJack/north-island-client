#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_QUIZ_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_QUIZ_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "client/subsystems/minigames/QuizGame.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Q&A activity's screen: timer bar, question and answers, host, player's figure, lives and best players' names over a QuizGame.
    /// @details Original: QuizWindow.xml, TConundrumGame's drawing.
    class QuizWindow {
        public:
            /// @brief Loads the form, background picture and wires the painter, click and Leave handlers.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Hooks handed to the QuizGame.
            /// @param ownLook Gives the player's current look.
            QuizWindow(
                GameServices& services,
                ui::Screen& screen,
                QuizGame::Hooks hooks,
                std::function<const CharacterLook&()> ownLook
            );

            /// @brief Tells whether the quiz form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives the game state.
            /// @return The QuizGame.
            QuizGame& game() { return m_game; }

            /// @brief Shows the form.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Draws the whole screen: timer, player, question, host, lives, names and result.
            /// @details Original: TConundrumGame.DrawControl.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the question, the answers, their boxes and marks.
            /// @details Original: DrawConundrumQuestion.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void drawQuestion(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the timer bar and one segment per second left.
            /// @details Original: DrawConundrumCountdown.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void drawCountdown(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws a small face for each life left.
            /// @details Original: DrawConundrumRoles.
            /// @param canvas Canvas to draw on.
            void drawLives(gfx::Canvas& canvas);

            /// @brief Draws the names of the players still in, the own one in yellow.
            /// @details Original: DrawConundrumParticipants.
            /// @param canvas Canvas to draw on.
            void drawParticipants(gfx::Canvas& canvas);

            /// @brief Draws the sign of the answer by the host.
            /// @details Original: DrawConundrumAnswerResult.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in milliseconds.
            void drawResult(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws a named picture if it exists.
            /// @param canvas Canvas to draw on.
            /// @param name Picture name.
            /// @param at Top-left position.
            void drawPicture(gfx::Canvas& canvas, const char* name, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            std::function<const CharacterLook&()> m_ownLook;
            QuizGame m_game;
            ui::Element& m_form;
            gfx::PictureId m_background;
            const ui::Spot& m_timerBarSpot;
            const ui::Spot& m_timerSecondsSpot;
            const ui::Spot& m_playerSpot;
            const ui::Spot& m_hostSpot;
            const ui::Spot& m_livesSpot;
            const ui::Spot& m_participantsSpot;
            const ui::Spot& m_answersSpot;
            const ui::Spot& m_boxSpot;
            const ui::Spot& m_yellowSpot;
            const ui::Spot& m_whiteSpot;
            FigureAnimation m_animation;
            bool m_selectionBox = false;
    };

}

#endif
