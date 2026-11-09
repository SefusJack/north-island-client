#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SHEEP_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SHEEP_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <random>

namespace client {

    /// @brief The "Sheep Dream" minigame: hammer the good dreams of a sleeper and spare the bad ones.
    /// @details Original: TH_MiniGame.pas Tsport_SheepDream, game kind 5 (kind 1 the sheep, 2 the pig); 30 seconds, 3 hearts.
    class SheepGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds

            /// @brief Callbacks the game needs from its host (frame count of the sleeper's sprite).
            struct Hooks : GameHooks {
                std::function<int(int slot)> sleeperFrames;
            };

            /// @brief Creates the game.
            /// @param hooks Host callbacks.
            /// @param gameKind 1 the sheep, otherwise the pig (the packet's level byte).
            SheepGame(
                Hooks hooks,
                int gameKind
            );

            /// @brief Starts a game (resets state).
            /// @details Original: Tsport_SheepDream.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Begins play after the instructions.
            /// @details Original: FStarted and StartSheepDreamGame.
            /// @param nowMs Current time in ms.
            void begin(std::uint64_t nowMs);

            /// @brief Leaves the game.
            /// @details Original: ExitSheepDreamGame (Leave on the instructions, the Exit button).
            void leave();

            /// @brief Handles a click (either button): a click on the dream hits it.
            /// @details Original: SheepDreamMouseDown.
            /// @param at Screen point.
            void clicked(core::Point at);

            /// @brief Runs the frames that have passed.
            /// @details Original: UpdateSheepDreamGame and DrawSheepDreamGame.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

            /// @brief The game kind.
            /// @return 1 the sheep, otherwise the pig.
            int gameKind() const { return m_gameKind; }

            /// @brief Whether play has started.
            /// @return True after Begin.
            bool started() const { return m_started; }

            /// @brief Hearts left.
            /// @return The heart count.
            int lives() const { return m_lives; }

            /// @brief Points scored.
            /// @return The score.
            int score() const { return m_score; }

            /// @brief Seconds left.
            /// @return The remaining time.
            int timeLeft() const { return m_timeLeft; }

            /// @brief Whether the game is over.
            /// @return True when over.
            bool gameOver() const { return m_gameOver; }

            /// @brief The dream in the bubble.
            /// @details Original: FDreamKind mod 4.
            /// @return The dream kind 0..3.
            int dreamKind() const { return m_dreamKind % 4; }

            /// @brief How far the dream still has to rise.
            /// @return 0 to 100.
            int dreamRise() const { return m_dreamRise; }

            /// @brief Fraction of the way from the last frame to the next.
            /// @param nowMs Current time in ms.
            /// @return 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief The dream's rise drawn part of the way from where the frame before had it.
            /// @param amount Fraction 0 to 1 between the frames.
            /// @return The interpolated rise (a new dream is at its start at once).
            int dreamRise(double amount) const;

            /// @brief Whether the dream was hit.
            /// @return True when hit.
            bool dreamHit() const { return m_dreamHit; }

            /// @brief The sleeper's blink frame.
            /// @details Original: the second frame of Sleepbabo for the half second after each second's start.
            /// @return 0 or 1.
            int blinkFrame() const { return m_blinkFrame; }

            /// @brief Frame of the hammer.
            /// @return The frame index.
            int hammerFrame() const { return m_hammerFrame; }

            /// @brief Frame of the hit's flash.
            /// @return 0 to 4, or -1 for none.
            int effectFrame() const { return m_effectActive ? m_effectFrame : -1; }

            /// @brief Action slot of the sleeper's figure.
            /// @details Original: the game's FDirection.
            /// @return The slot.
            int sleeperSlot() const { return m_sleeperSlot; }

            /// @brief Frame of the sleeper's figure.
            /// @return The frame index.
            int sleeperFrame() const { return m_sleeperFrame; }

        private:
            /// @brief Runs one logic frame.
            /// @details Original: UpdateSheepDreamGame.
            /// @param nowMs Current time in ms.
            void frame(std::uint64_t nowMs);

            /// @brief Moves the hammer and brings up a new dream when due.
            /// @details Original: UpdateSheepDreamTarget.
            void updateTarget();

            /// @brief Advances the hit's flash.
            /// @details Original: UpdateSheepDreamEffect.
            void updateEffect();

            /// @brief Sets the blink and the sleeper's pose.
            /// @details Original: DrawSheepDreamScene.
            /// @param nowMs Current time in ms.
            void drawStep(std::uint64_t nowMs);

            /// @brief Random number below a limit.
            /// @param count Exclusive upper limit.
            /// @return A value from 0 to count - 1.
            int random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

            Hooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            int m_gameKind;
            std::uint64_t m_lastFrameMs = 0;
            std::uint64_t m_secondMs = 0;
            bool m_started = false;
            bool m_gameOver = false;
            int m_gameOverDelay = 0;
            int m_lives = 3; // hearts (lives) at the start
            int m_score = 0;
            int m_timeLeft = 30; // seconds on the clock at the start
            int m_dreamKind = 0;
            int m_dreamTick = 0;
            int m_dreamRise = 0;
            int m_previousDreamRise = 0;
            bool m_dreamHit = false;
            int m_hammerTick = 10; // frames since the hammer swing; starts at the swing length (idle)
            int m_hammerFrame = 0;
            bool m_effectActive = false;
            int m_effectTick = 0;
            int m_effectFrame = 0;
            int m_blinkFrame = 0;
            int m_sleeperSlot = 26; // sprite slot (action) 26: the sleeping pose
            int m_sleeperFrame = 0;
            HumanFrame m_sleeperAnim;
            bool m_finished = false;
    };

}

#endif
