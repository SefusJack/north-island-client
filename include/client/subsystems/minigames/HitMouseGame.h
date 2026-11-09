#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HIT_MOUSE_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HIT_MOUSE_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Point.h"

#include <array>
#include <cstdint>
#include <random>

namespace client {

    /// @brief The hit-the-mouse game (original CH_MiniGame.pas CH_THitMouseGame without drawing; game kind 3): seven holes with targets and bombs, a mallet; 40 seconds, 30 points win.
    class HitMouseGame {
        public:
            static constexpr int kHoles = 7; // mouse holes on the board
            static constexpr int kStartSeconds = 40; // seconds on the clock at the start
            static constexpr int kPointsToWin = 30; // score needed to win
            static constexpr int kTargetFrames = 5; // frames of a mouse popping up

            /// @brief What the targets look like (original CH_THitMouseGame.FPicIndexes.B[144]), chosen by the NPC that started the game.
            enum class Look { Mouse, Rabbit, Turtle };

            /// @brief State of the mallet (original FHammerState 0 ready, 1 coming up, 2 down).
            enum class Hammer { Ready, Up, Down };

            /// @brief One hole and what is in it (original FHoleStates, FHoleAnimFrames, FHoleHitFrames, FHolePos).
            struct Hole {
                bool bomb = false;
                int state = 0;
                int animFrame = 0;
                int hitFrame = 0;
                std::uint64_t tickMs = 0;
                core::Point position;
            };

            /// @brief Where the mallet struck and what it found.
            struct Strike {
                /// @brief What the mallet hit.
                enum class Kind { None, Target, Bomb };
                Kind kind = Kind::None;
                core::Point at;
            };

            /// @brief Creates the game with its callbacks.
            /// @param hooks Callbacks into the client.
            explicit HitMouseGame(
                GameHooks hooks
            );

            /// @brief Starts the game; the NPC that started it decides how it looks (original CH_THitMouseGame.Create).
            /// @param npcId NPC that started the game.
            /// @param nowMs Current time in ms.
            void open(int npcId, std::uint64_t nowMs);

            /// @brief Begin on the instructions (original FPicIndexes.B[120]).
            void begin() { m_begun = true; }

            /// @brief One update of the game by the clock (original UpdateHitMouseGame).
            /// @param nowMs Current time in ms.
            void step(std::uint64_t nowMs);

            /// @brief A mouse button came up at a screen point: the mallet strikes (original HitMouseMouseUp).
            /// @param at Screen point.
            /// @param nowMs Current time in ms.
            /// @return What the mallet found and where.
            Strike released(core::Point at, std::uint64_t nowMs);

            /// @brief The Leave button (original LeaveGameClick, FinishHitMouseGame(0)).
            void leave();

            /// @brief How the targets look.
            /// @return The look.
            Look look() const { return m_look; }

            /// @brief Whether the game has begun.
            /// @return True after begin().
            bool begun() const { return m_begun; }

            /// @brief Seconds left.
            /// @return Seconds.
            int timeLeft() const { return m_timeLeft; }

            /// @brief Current score.
            /// @return Points.
            int score() const { return m_score; }

            /// @brief Game state (original FGameState).
            /// @return 0 counting down, 1 playing, 2 the time is out, 3 over.
            int state() const { return m_state; }

            /// @brief The countdown shown (original FPicIndexes.V[14]).
            /// @return 3, 2, 1, 0 for "Go", below 0 nothing.
            int countdown() const { return m_countdown; }

            /// @brief State of the mallet.
            /// @return The mallet state.
            Hammer hammer() const { return m_hammer; }

            /// @brief A hole.
            /// @param index Hole number 1..7.
            /// @return The hole.
            const Hole& hole(int index) const { return m_holes[static_cast<std::size_t>(index)]; }

        private:
            /// @brief Sets target hold times and frame intervals by the time left (original SetHitMouseSpeeds).
            void setSpeeds();

            /// @brief Lets more targets or bombs come up while few holes are busy (original SpawnHitMouseTargets).
            void spawn();

            /// @brief Animates the holes' targets (original AnimateHitMouseTargets).
            /// @param nowMs Current time in ms.
            void animate(std::uint64_t nowMs);

            /// @brief Counts the seconds down while playing (original TickHitMouseTimer).
            /// @param nowMs Current time in ms.
            void tickTimer(std::uint64_t nowMs);

            /// @brief Moves the mallet from down to up to ready (original UpdateHammerCursor).
            /// @param nowMs Current time in ms.
            void updateHammer(std::uint64_t nowMs);

            /// @brief Runs the 3, 2, 1, Go countdown (original HitMouseCountdown).
            /// @param nowMs Current time in ms.
            void runCountdown(std::uint64_t nowMs);

            /// @brief Ends the game some time after the time is out (original CheckHitMouseEnd).
            /// @param nowMs Current time in ms.
            void checkEnd(std::uint64_t nowMs);

            /// @brief Tests whether a screen point is in a target's or bomb's box (original HitMouseHitTest).
            /// @param hole The hole.
            /// @param at Screen point.
            /// @return True on a hit.
            bool hits(const Hole& hole, core::Point at) const;

            /// @brief Stops the music and finishes the game (original FinishHitMouseGame).
            /// @param result 1 won, 0 lost.
            void finish(int result);

            /// @brief Whether an interval has passed since a time.
            /// @param since Start time in ms.
            /// @param interval Interval in ms.
            /// @param nowMs Current time in ms.
            /// @return True when the interval has passed.
            bool elapsed(std::uint64_t since, std::uint64_t interval, std::uint64_t nowMs) const { return nowMs - since >= interval; }

            /// @brief Random number below a count.
            /// @param count Exclusive upper bound.
            /// @return A value from 0 to count - 1.
            int random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

            GameHooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            Look m_look = Look::Rabbit;
            std::array<Hole, kHoles + 1> m_holes;
            std::array<int, kHoles + 1> m_holdMs{};
            std::array<int, kHoles + 1> m_intervalMs{};
            int m_timeLeft = kStartSeconds;
            int m_countdown = 4; // countdown start: 4 steps down to 3, 2, 1, 0 (Go)
            int m_state = 0;
            int m_score = 0;
            bool m_begun = false;
            Hammer m_hammer = Hammer::Ready;
            std::uint64_t m_hammerMs = 0;
            std::uint64_t m_timerMs = 0;
    };

}

#endif
