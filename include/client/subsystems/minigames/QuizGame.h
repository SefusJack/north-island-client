#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_QUIZ_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_QUIZ_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"
#include "core/Point.h"

#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <string>

namespace client {

    /// @brief The Q&A (conundrum) activity without its drawing.
    /// @details Original: HL_Minigame.pas TConundrumGame; server messages 57/8; questions with four answers, six seconds to answer.
    class QuizGame {
        public:
            static constexpr int kAnswers = 4; // answer choices per question, 1-based
            static constexpr int kMostPlayers = 20; // most players in a round, 1-based
            static constexpr int kStartLives = 6; // lives at the start
            static constexpr int kQuestionSeconds = 6; // seconds to answer a question

            /// @brief Result of an answer as the server reports it.
            /// @details Original: packet 57/8/5.
            enum class Result { None = 0, Right = 1, Wrong = 2, NoAnswer = 3 };

            /// @brief Callbacks the quiz needs from its host (chat, names, ids, screen enter/leave).
            struct Hooks : GameHooks {
                std::function<void(const std::string&, bool byPlayer)> chat;
                std::function<std::string(std::int32_t)> playerName;
                std::function<std::int32_t()> ownId;
                std::function<int()> mapId;
                std::function<void()> enterScreen;
                std::function<void()> leaveScreen;
            };

            /// @brief Creates the quiz.
            /// @param hooks Host callbacks.
            explicit QuizGame(
                Hooks hooks
            );

            /// @brief Resets all state.
            /// @details Original: ResetConundrumGame.
            /// @param nowMs Current time in ms.
            void reset(std::uint64_t nowMs);

            /// @brief Handles a 57/8 server packet.
            /// @details Original: RecvConundrumPacket.
            /// @param payload Packet bytes (sub id at offset 2).
            /// @param nowMs Current time in ms.
            void receive(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Finds which answer the mouse is over.
            /// @details Original: HitTestConundrumAnswer.
            /// @param mouse Screen point.
            void hover(core::Point mouse);

            /// @brief Handles the mouse button going up: picks an answer and sends 57/8/3.
            /// @details Original: ConundrumAnswerClick.
            /// @param mouse Screen point.
            void clicked(core::Point mouse);

            /// @brief Asks the server to let the player leave (57/8/2).
            /// @details Original: RequestLeaveConundrum.
            void leaveRequested();

            /// @brief Asks to leave when the player is not on the Q&A map.
            /// @details Original: DrawControl; the screen is only for the Q&A map, off it the server is asked again and again.
            void leaveWhenOffMap();

            /// @brief Seconds of the countdown bar to show; plays the tick sounds.
            /// @details Original: DrawConundrumCountdown; ticks at 2, 1 and 0.
            /// @param nowMs Current time in ms.
            /// @return Seconds left (0: none).
            int secondsLeft(std::uint64_t nowMs);

            /// @brief Advances the host's blinking.
            /// @details Original: DrawConundrumHost, after the host is drawn.
            /// @param nowMs Current time in ms.
            void stepHost(std::uint64_t nowMs);

            /// @brief Whether the answer result is on show.
            /// @return True while shown.
            bool resultOn() const { return m_resultOn; }

            /// @brief Counts the result's three seconds, then waits for the next question.
            /// @details Original: DrawConundrumAnswerResult; call after the result's picture is drawn.
            /// @param nowMs Current time in ms.
            void stepResult(std::uint64_t nowMs);

            /// @brief Dots of "Waiting for questions..." and of the place waiting.
            /// @details Original: DrawConundrumQuestion.
            /// @param nowMs Current time in ms.
            /// @return Dot count 0..2.
            int dots(std::uint64_t nowMs);

            /// @brief Whether the yellow selection box shows on this frame (a tenth of a second after the last).
            /// @details Original: DrawConundrumQuestion.
            /// @param nowMs Current time in ms.
            /// @return True when the box is due.
            bool selectionBoxDue(std::uint64_t nowMs);

            /// @brief Plays the sound for a picked answer, once per result.
            /// @details Original: DrawConundrumQuestion's sounds.
            void soundResult();

            /// @brief The question to show.
            /// @return Pending question, else the kept one.
            const std::string& questionText() const { return m_question.empty() ? m_shownQuestion : m_question; }

            /// @brief The answer text to show.
            /// @param answer Answer number 1..4.
            /// @return The text (empty when out of range).
            const std::string& answerText(int answer) const;

            /// @brief Lives left.
            /// @return The life count.
            int lives() const { return m_lives; }

            /// @brief The answer the player selected.
            /// @return Answer number, 0 for none.
            int selected() const { return m_selected; }

            /// @brief The right answer the server named.
            /// @return Answer number, 0 for none.
            int picked() const { return m_picked; }

            /// @brief Result of the last answer.
            /// @return The result.
            Result result() const { return m_result; }

            /// @brief The answer under the mouse.
            /// @return Answer number, 0 for none.
            int hoverAnswer() const { return m_hover; }

            /// @brief Whether no life is left.
            /// @return True when lost.
            bool lost() const { return m_lost; }

            /// @brief Whether the game is over.
            /// @return True when over.
            bool over() const { return m_over; }

            /// @brief Whether the final places were listed.
            /// @return True when listed.
            bool resultsListed() const { return m_resultsListed; }

            /// @brief Whether the own name is missing from the final list (B[76]).
            /// @return True when not listed.
            bool notListed() const { return m_notListed; }

            /// @brief The host's frame.
            /// @details Original: the host human's FAnimFrame (0 plain, 1 blinking, 3 pleased, 4 sorry).
            /// @return The frame number.
            int hostState() const { return m_hostState; }

            /// @brief Number of listed places.
            /// @return The count.
            int playerCount() const { return m_playerCount; }

            /// @brief Name at a place of the list.
            /// @param place Place number 1..kMostPlayers.
            /// @return The player's name.
            const std::string& playerName(int place) const { return m_playerNames[static_cast<std::size_t>(place)]; }

            /// @brief Whether a place holds the own name.
            /// @param place Place number.
            /// @return True for the own place.
            bool isOwnPlace(int place) const;

        private:
            /// @brief Makes the pending question and answers the shown ones.
            /// @details Original: FShownQuestion, FAnswerTexts.
            void keepQuestion();

            /// @brief Random host blink wait.
            /// @return Seconds from 7 to 11.
            int randomSeconds();

            Hooks m_hooks;
            std::string m_question;
            std::array<std::string, kAnswers + 1> m_answers;
            std::string m_shownQuestion;
            std::array<std::string, kAnswers + 1> m_answerTexts;
            std::array<std::string, kMostPlayers + 1> m_playerNames;
            int m_playerCount = 0;
            int m_lives = kStartLives;
            bool m_questionReceived = false;
            int m_selected = 0;
            int m_picked = 0;
            Result m_result = Result::None;
            int m_hover = 0;
            int m_questionId = 0;
            bool m_lost = false;
            bool m_resultSoundPending = false;
            bool m_resultOn = false;
            bool m_over = false;
            bool m_notListed = false;
            bool m_resultsListed = false;
            bool m_lifeGained = false;
            std::uint64_t m_countdownMs = 0;
            std::uint64_t m_resultMs = 0;
            std::uint64_t m_hostWaitMs = 0;
            std::uint64_t m_hostSinceMs = 0;
            std::uint64_t m_flashMs = 0;
            std::uint64_t m_dotsMs = 0;
            int m_dots = 0;
            int m_tickSound = 2;
            int m_hostState = 0;
            std::mt19937 m_random{std::random_device{}()};
    };

}

#endif
