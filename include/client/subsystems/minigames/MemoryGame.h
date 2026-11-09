#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_MEMORY_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_MEMORY_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Point.h"

#include <array>
#include <cstdint>
#include <random>

namespace client {

    /// @brief The memory game: numbers show, then hide under circles to be clicked from the smallest up.
    /// @details Original: CJ_MiniGame.pas TSport_MemoryGame without its drawing; game kind 15; twelve rounds win, three lives, ten seconds a round.
    class MemoryGame {
        public:
            static constexpr int kMostCards = 10; // most number cards in a round
            static constexpr int kCardWidth = 38; // card width, px
            static constexpr int kCardHeight = 37; // card height, px
            static constexpr int kStartLives = 3; // lives at the start
            static constexpr int kRoundsToWin = 12; // rounds cleared to win

            /// @brief One number card and its place on the board.
            struct Card {
                int number = 0;
                int x = 0;
                int y = 0;
                bool revealed = true;
            };

            /// @brief What a frame is to show.
            /// @details Original: the decisions DrawMemoryGame takes as it draws.
            struct Frame {
                bool numbersOnly = false;
                bool circles = false;
                int countdown = -1;
                bool rightSign = false;
                bool wrongSign = false;
                bool win = false;
                bool lose = false;
            };

            /// @brief Creates the game.
            /// @param hooks Callbacks to the host.
            explicit MemoryGame(
                GameHooks hooks
            );

            /// @brief Opens the game.
            /// @param nowMs Current time in ms.
            /// @details Original: TSport_MemoryGame.Create.
            void open(std::uint64_t nowMs);

            /// @brief Shows the form from its Begin.
            /// @details Original: MemoryGameForm.ShowCtrl.
            void begin() { m_formShown = true; }

            /// @brief Runs the game's update, then takes the draw's decisions.
            /// @param nowMs Current time in ms.
            /// @param instructionsOpen Whether the instructions are showing.
            /// @return What the frame is to show.
            /// @details Original: UpdateMemoryGame, then DrawMemoryGame's decisions.
            Frame step(std::uint64_t nowMs, bool instructionsOpen);

            /// @brief Handles the left button going down at a screen point.
            /// @param point Screen point.
            /// @param nowMs Current time in ms.
            /// @details Original: MemoryGameMouseDown.
            void clicked(core::Point point, std::uint64_t nowMs);

            /// @brief Leaves the game.
            /// @details Original: ExitMemoryGame.
            void leave();

            /// @brief Whether the form is shown.
            /// @return True when shown.
            bool formShown() const { return m_formShown; }

            /// @brief Lives left.
            /// @return The count.
            int lives() const { return m_lives; }

            /// @brief Rounds won.
            /// @return The score.
            int score() const { return m_score; }

            /// @brief Seconds left in the round.
            /// @return The time.
            int timeLeft() const { return m_timeLeft; }

            /// @brief Number of cards laid out so far (1..cardCount()).
            /// @return The count.
            int cardCount() const { return m_nextCard - 1; }

            /// @brief Number of card places chosen so far.
            /// @return The count.
            int positionCount() const { return m_nextPos - 1; }

            /// @brief A card by its number.
            /// @param index Card number from 1.
            /// @return The card.
            const Card& card(int index) const { return m_cards[static_cast<std::size_t>(index)]; }

        private:
            /// @brief Starts a new round.
            /// @param nowMs Current time in ms.
            /// @details Original: ResetMemoryGame.
            void reset(std::uint64_t nowMs);

            /// @brief Draws the numbers and finds places for the cards.
            /// @details Original: GenerateMemoryCards.
            void generate();

            /// @brief Sorts the cards' numbers ascending.
            /// @details Original: SortMemoryCards.
            void sortCards();

            /// @brief Steps the 3, 2, 1, Go countdown.
            /// @param nowMs Current time in ms.
            /// @details Original: MemoryGameCountdown.
            void countdownStep(std::uint64_t nowMs);

            /// @brief Counts the round's seconds down.
            /// @param nowMs Current time in ms.
            /// @details Original: TickMemoryCountdown.
            void tick(std::uint64_t nowMs);

            /// @brief Whether an interval has passed.
            /// @param since Start time in ms.
            /// @param interval Interval in ms.
            /// @param nowMs Current time in ms.
            /// @return True when the interval is over.
            bool elapsed(std::uint64_t since, std::uint64_t interval, std::uint64_t nowMs) const { return nowMs - since >= interval; }

            /// @brief Ends the game and reports the result.
            /// @param result The result sent to the server.
            /// @details Original: FinishMemoryGame.
            void finish(int result);

            /// @brief Random integer below a count.
            /// @param count Exclusive upper bound.
            /// @return A value in 0 to count-1.
            int random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

            GameHooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            std::array<Card, kMostCards> m_cards;
            int m_nextCard = 1;
            int m_expected = 1;
            int m_level = 1;
            int m_nextPos = 1;
            int m_countdown = 4;
            int m_phase = 2;
            std::uint64_t m_resultMs = 0;
            std::uint64_t m_roundOkMs = 0;
            std::uint64_t m_missMs = 0;
            int m_timeLeft = 10; // seconds left in the round (10 at the start)
            std::uint64_t m_countdownMs = 0;
            std::uint64_t m_showCardsMs = 0;
            bool m_showCards = true;
            bool m_missed = false;
            bool m_roundCleared = false;
            bool m_missSoundPending = false;
            bool m_revealOnMiss = true;
            bool m_won = false;
            int m_score = 0;
            int m_lives = kStartLives;
            bool m_musicPending = false;
            bool m_formShown = false;
    };

}

#endif
