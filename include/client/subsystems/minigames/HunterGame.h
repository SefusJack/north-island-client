#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HUNTER_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_HUNTER_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/HunterRole.h"
#include "core/Point.h"
#include "core/Size.h"
#include "formats/NpcInfo.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>

namespace client {

    /// @brief The hunting game: animals run about, clicks shoot them, attackers cost hearts.
    /// @details Original: TH_Hunter.pas TSport_Hunter, game kind 4; ticks every 30 ms; only the result is sent to the server.
    class HunterGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kMostRoles = 50; // most targets (bees, pigs) on the field at once, 1-based

            /// @brief Callbacks the game uses to reach the NPC table, sprites and player data.
            struct Hooks : GameHooks {
                std::function<std::optional<formats::NpcInfo>(int npcId)> npcInfo;
                std::function<core::Size(const formats::NpcInfo&, int slot)> spriteSize;
                std::function<int(const formats::NpcInfo&, int slot)> frameCount;
                std::function<bool(const formats::NpcInfo&, core::Point feet, int slot, int frame, int scale, core::Point point)> hit;
                std::function<int()> playerBodyType;
            };

            /// @brief Creates the game with its callbacks.
            /// @param hooks Callbacks to the host.
            explicit HunterGame(
                Hooks hooks
            );

            /// @brief Opens the game on its instructions.
            /// @param nowMs Current time in ms.
            /// @details Original: TSport_Hunter.Create.
            void open(std::uint64_t nowMs);

            /// @brief Starts play from the instructions.
            /// @param nowMs Current time in ms.
            /// @details Original: FGameStarted and StartHunterGame.
            void begin(std::uint64_t nowMs);

            /// @brief Leaves the game (Leave on the instructions, the Exit button).
            /// @details Original: ExitHunterGame.
            void leave();

            /// @brief Fires a shot at a point.
            /// @param at Mouse position.
            /// @details Original: HunterMouseDown with the left button.
            void shoot(core::Point at);

            /// @brief Moves the sight to where the mouse moved.
            /// @param to New sight position.
            /// @details Original: HunterMouseMove.
            void moveAim(core::Point to) { m_aim = to; }

            /// @brief Runs the frames that have passed.
            /// @param nowMs Current time in ms.
            /// @param mouse Mouse position, for the animals it is over.
            /// @details Original: UpdateHunterGame and DrawHunterGame.
            void update(std::uint64_t nowMs, core::Point mouse);

            /// @brief Whether play has started.
            /// @return True after begin().
            bool started() const { return m_started; }

            /// @brief Hearts left.
            /// @return Lives remaining.
            int lives() const { return m_lives; }

            /// @brief Time left.
            /// @return Seconds remaining.
            int timeLeft() const { return m_timeLeft; }

            /// @brief Current score.
            /// @return Five points per animal shot.
            int score() const { return m_kills * 5; }

            /// @brief The animals by their number from 1 (empty: none, or freed).
            /// @return The role slots.
            const std::array<std::optional<HunterRole>, kMostRoles + 1>& roles() const { return m_roles; }

            /// @brief How far time has come from the last frame towards the next, for drawing animals between frames.
            /// @param nowMs Current time in ms.
            /// @return A fraction from 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Number of animals created so far.
            /// @return The role count.
            int roleCount() const { return m_roleCount; }

            /// @brief How far the screen is shaken (zero unless an animal is at the player).
            /// @return The shake offset.
            core::Point shake() const { return m_shaking ? m_shake : core::Point{0, 0}; }

            /// @brief Where the sight is.
            /// @return The aim position.
            core::Point aim() const { return m_aim; }

            /// @brief Where the flash of the shot is.
            /// @return The shot position.
            core::Point shotAt() const { return m_shot; }

            /// @brief Frame of the shot flash (1 to 4; 0: none).
            /// @return The effect frame.
            int shotFrame() const { return m_shotFired ? m_effectFrame : 0; }

        private:
            /// @brief Runs one logic step.
            /// @param nowMs Current time in ms.
            void updateStep(std::uint64_t nowMs);

            /// @brief Runs one draw-time step.
            /// @param nowMs Current time in ms.
            void drawStep(std::uint64_t nowMs);

            /// @brief Creates animals up to the limit.
            /// @param nowMs Current time in ms.
            void spawnTargets(std::uint64_t nowMs);

            /// @brief Creates one animal in a slot.
            /// @param index Role number.
            /// @param nowMs Current time in ms.
            void createRole(int index, std::uint64_t nowMs);

            /// @brief Moves an animal one step.
            /// @param role The animal.
            /// @param nowMs Current time in ms.
            void moveAnimal(HunterRole& role, std::uint64_t nowMs);

            /// @brief Turns an animal towards its destination.
            /// @param role The animal.
            void faceDestination(HunterRole& role) const;

            /// @brief Advances the screen shake.
            void updateShake();

            /// @brief Updates the animals' drawn state.
            /// @param nowMs Current time in ms.
            void drawAnimals(std::uint64_t nowMs);

            /// @brief Updates the sight and shot flash.
            void drawCrosshair();

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
            std::array<std::optional<HunterRole>, kMostRoles + 1> m_roles;
            int m_roleCount = 0;
            bool m_started = false;
            bool m_finished = false;
            bool m_gameOver = false;
            int m_lives = 3; // lives at the start
            int m_timeLeft = 30; // seconds on the clock at the start
            int m_endDelay = 100; // frames (30 ms each) between the end and sending the result
            int m_tickCount = 0;
            int m_kills = 0;
            bool m_shaking = false;
            int m_shakePhase = 0;
            core::Point m_shake;
            core::Point m_aim;
            core::Point m_mouse;
            core::Point m_shot;
            bool m_shotFired = false;
            int m_effectFrame = 0;
    };

}

#endif
