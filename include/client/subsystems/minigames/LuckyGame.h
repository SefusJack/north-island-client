#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_LUCKY_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_LUCKY_GAME_H

#include "client/subsystems/minigames/FallingObject.h"
#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"
#include "core/Size.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <string>

namespace client {

    /// @brief The catching game "Luckey": coins and bombs fall, the player's figure follows the mouse.
    /// @details Original: MK_MiniGame.pas TLuckey, game kind 13; thirty seconds, three hearts, thirty coins win; only the result goes to the server.
    class LuckyGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kObjects = 51; // most falling objects (coins, bombs...) at once

            /// @brief The phase of the game.
            /// @details Original: FPicIndexes.V[24]: 2 the countdown to the start, 3 playing, 0 over.
            enum class Phase { Over = 0, Countdown = 2, Playing = 3 };

            /// @brief Callbacks the game uses to reach pictures, effects and figures.
            struct Hooks : GameHooks {
                std::function<core::Size(const std::string&)> pictureSize;
                std::function<void(core::Point)> bombLight;
                std::function<int(int slot)> playerFrames;
                std::function<int(int slot)> throwerFrames;
            };

            /// @brief Creates the game.
            /// @param hooks Callbacks to the host.
            /// @param npcId NPC of the thrower; picks its sprite.
            /// @param lastDropX Where the last game's drop ended (LastLuckyPosX, which outlives a game).
            /// @details The thrower is sprite 206 for NPC 10000, 483 for any other (+ 1000 for the bank's number).
            LuckyGame(
                Hooks hooks,
                int npcId,
                int lastDropX
            );

            /// @brief Opens the game; the points are not asked for.
            /// @param nowMs Current time in ms.
            /// @details Original: TLuckey.Create.
            void open(std::uint64_t nowMs);

            /// @brief Handles the form's Quit button.
            /// @details Original: ExitLuckeyGame.
            void quitClicked();

            /// @brief Runs the frames that have passed.
            /// @param nowMs Current time in ms.
            /// @param instructionsOpen Whether the instructions are showing.
            /// @param mouseX Mouse x for the player's figure to follow.
            /// @details Original: UpdateLuckeyGame and DrawLuckeyGame.
            void update(std::uint64_t nowMs, bool instructionsOpen, int mouseX);

            /// @brief Whether the game form is shown.
            /// @return True when shown.
            bool formShown() const { return m_formShown; }

            /// @brief The current phase.
            /// @return The phase.
            Phase phase() const { return m_phase; }

            /// @brief Coins caught.
            /// @return The score.
            int score() const { return m_score; }

            /// @brief Hearts left.
            /// @return The count.
            int hearts() const { return m_hearts; }

            /// @brief Seconds left.
            /// @return The time.
            int timeLeft() const { return m_timeLeft; }

            /// @brief The countdown before the start.
            /// @return The countdown value.
            int countdown() const { return m_countdown; }

            /// @brief Where the last drop was.
            /// @return The x position.
            int lastDropX() const { return m_lastDropX; }

            /// @brief The time of the frame last run.
            /// @return Time in ms.
            std::uint64_t lastFrameMs() const { return m_lastFrameMs; }

            /// @brief Sprite number of the thrower.
            /// @return The sprite.
            int throwerSprite() const { return m_throwerSprite; }

            /// @brief Where the player's figure stands.
            /// @return The position.
            core::Point playerAt() const { return {m_playerX, kPlayerY}; }

            /// @brief Where the thrower stands.
            /// @return The position.
            core::Point throwerAt() const { return {m_throwerX, kThrowerY}; }

            /// @brief How far time has come from the last frame towards the next, for drawing between frames.
            /// @param nowMs Current time in ms.
            /// @return A fraction from 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Where the player's figure stood before the last frame.
            /// @return The position.
            core::Point previousPlayerAt() const { return {m_previousPlayerX, kPlayerY}; }

            /// @brief Where the thrower stood before the last frame.
            /// @return The position.
            core::Point previousThrowerAt() const { return {m_previousThrowerX, kThrowerY}; }

            /// @brief Action slot of the player's figure (the game's FDirection).
            /// @return The slot.
            int playerSlot() const { return m_playerSlot; }

            /// @brief Action slot of the thrower.
            /// @return The slot.
            int throwerSlot() const { return m_throwerSlot; }

            /// @brief Frame of the player's figure.
            /// @return The frame.
            int playerFrame() const { return m_playerFrame; }

            /// @brief Frame of the thrower.
            /// @return The frame.
            int throwerFrame() const { return m_throwerFrame; }

            /// @brief The falling objects.
            /// @return The object slots.
            const std::array<FallingObject, kObjects>& objects() const { return m_objects; }

            /// @brief The frame of an object that the last draw showed.
            /// @param index Object index.
            /// @return The frame.
            int shownFrame(int index) const { return m_shownFrames[static_cast<std::size_t>(index)]; }

            /// @brief The words over the screen when it is over ("Game Over", "You Win", "You Lose").
            /// @return The text.
            std::string overText() const;

        private:
            static constexpr int kPlayerY = 550; // y of the player on the 800 x 600 field, px
            static constexpr int kThrowerY = 150; // y of the thrower on the field, px

            /// @brief Runs one frame.
            /// @param nowMs Current time in ms.
            /// @param instructionsOpen Whether the instructions are showing.
            /// @param mouseX Mouse x.
            void frame(std::uint64_t nowMs, bool instructionsOpen, int mouseX);

            /// @brief Steps the countdown before the start.
            /// @param nowMs Current time in ms.
            void countdownStep(std::uint64_t nowMs);

            /// @brief Moves the player's figure towards the mouse.
            /// @param mouseX Mouse x.
            void moveToMouse(int mouseX);

            /// @brief Moves the thrower along the top.
            void bounceThrower();

            /// @brief Lets the objects fall and throws new ones.
            void fallObjects();

            /// @brief Checks which objects the player caught.
            void checkCatches();

            /// @brief Counts the game clock.
            /// @param nowMs Current time in ms.
            void tickClock(std::uint64_t nowMs);

            /// @brief Adds a falling object.
            /// @param kind Object kind.
            /// @param picture Picture name.
            /// @param frames Number of frames.
            void addObject(int kind, const char* picture, int frames);

            /// @brief Updates what the draw shows.
            /// @param nowMs Current time in ms.
            void drawStep(std::uint64_t nowMs);

            /// @brief Ends the game and reports the result.
            /// @param result The result sent to the server.
            void finish(int result);

            /// @brief Random integer below a count.
            /// @param count Exclusive upper bound.
            /// @return A value in 0 to count-1.
            int random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

            Hooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            std::uint64_t m_lastFrameMs = 0;
            std::uint64_t m_blinkMs = 0;
            std::uint64_t m_overMs = 0;
            bool m_formShown = false;
            Phase m_phase = Phase::Countdown;
            int m_score = 0;
            int m_hearts = 3; // hearts (lives) at the start
            int m_timeLeft = 30; // seconds on the clock at the start
            int m_countdown = 4; // start countdown, seconds
            int m_lastDropX = 0;
            int m_throwerSprite = 0;
            int m_playerX = 400; // player x, px (400 = middle of the 800 wide field)
            int m_throwerX = 400; // thrower x, px (field middle)
            int m_previousPlayerX = 400; // player x of the previous frame, px
            int m_previousThrowerX = 400; // thrower x of the previous frame, px
            int m_throwerDirection = 1;
            int m_playerSlot = 0;
            int m_throwerSlot = 5;
            int m_playerFrame = 0;
            int m_throwerFrame = 0;
            HumanFrame m_playerAnim;
            HumanFrame m_throwerAnim;
            std::array<FallingObject, kObjects> m_objects;
            std::array<int, kObjects> m_shownFrames{};
    };

}

#endif
