#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SCOTD_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SCOTD_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/ScotdRole.h"
#include "core/Point.h"
#include "core/Size.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace client {

    /// @brief The Scotd minigame: the player runs and jumps to hit NPCs that jump about the field.
    /// @details Original: TH_scotd.pas TSport_scotd, sport kind 2; the value of 57/1 is the NPC kind and 5 less the level is the heart count; 20 hits win.
    class ScotdGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kMaxRoles = 20; // most monsters on the field at once, 1-based
            static constexpr int kCatchesToWin = 20; // hits needed to win
            static constexpr int kGroundY = 500; // y of the ground line, px
            static constexpr int kFieldWidth = 800; // field width, px
            static constexpr int kFieldHeight = 600; // field height, px
            static constexpr int kEffectFrames = 4; // frames of the hit effect animation

            /// @brief Keys the game reacts to.
            /// @details Original: HandleSportKeyDown keys (arrows 37 to 40, Q 81, Space 32).
            enum class Key { Left, Right, Up, Down, Quit, Space };

            /// @brief Callbacks the game needs from its host (sprite sizes, frame counts, body type, random).
            struct Hooks : GameHooks {
                std::function<core::Size(bool player, int slot)> bodySize;
                std::function<int(bool player, int slot)> frameCount;
                std::function<int()> playerBodyType;
                std::function<int(int count)> random;
            };

            /// @brief Creates the game.
            /// @param hooks Host callbacks.
            explicit ScotdGame(
                Hooks hooks
            );

            /// @brief Starts a game.
            /// @details Original: TSport_scotd.Create; hearts are 5 less the level.
            /// @param nowMs Current time in ms.
            /// @param hearts Starting hearts.
            void open(std::uint64_t nowMs, int hearts);

            /// @brief Leaves the game; it ends as lost.
            /// @details Original: ExitScotdGame (the instructions' Leave, the exit button).
            void leave();

            /// @brief Starts the play to the music BGM0015.
            /// @details Original: the instructions' Begin (StartFormButtonClick 1), PlayScotdMusic.
            void begin();

            /// @brief Handles a mouse press: left steers to the point, right jumps.
            /// @details Original: ScotdMouseDown.
            /// @param button 0 left, 1 right.
            /// @param at Screen point.
            void mouseDown(int button, core::Point at);

            /// @brief Handles a key press.
            /// @details Original: ScotdKeyDown.
            /// @param key The key.
            void keyDown(Key key);

            /// @brief Runs the frames that have passed.
            /// @details Original: UpdateScotdGame, then DrawScotdGame.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

            /// @brief Whether play has started.
            /// @return True after Begin.
            bool started() const { return m_started; }

            /// @brief Hearts left.
            /// @return The heart count.
            int remaining() const { return m_remaining; }

            /// @brief NPCs hit so far.
            /// @return The catch count.
            int catchCount() const { return m_catchCount; }

            /// @brief The player's figure.
            /// @return The player role.
            const ScotdRole& player() const { return m_player; }

            /// @brief The NPC figures (index 0 unused).
            /// @return The role slots.
            const std::array<std::optional<ScotdRole>, kMaxRoles + 1>& roles() const { return m_roles; }

            /// @brief Fraction of the way from the last frame to the next.
            /// @param nowMs Current time in ms.
            /// @return 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Number of NPC slots in use.
            /// @return The role count.
            int roleCount() const { return m_roleCount; }

            /// @brief Whether the hit flash is shown.
            /// @details Original: DrawScotdCatchEffect.
            /// @return True when shown.
            bool effectShown() const { return m_effectShown; }

            /// @brief Frame of the hit flash.
            /// @return Frame 1 to 4.
            int effectFrame() const { return m_effectFrame; }

            /// @brief Where the hit flash is shown.
            /// @return The point.
            core::Point effectAt() const { return m_effectAt; }

        private:
            /// @brief Runs one logic step.
            /// @details Original: UpdateScotdGame.
            /// @param nowMs Current time in ms.
            void updateStep(std::uint64_t nowMs);

            /// @brief Runs one draw step (moves NPCs, sets what is drawn).
            /// @details Original: DrawScotdGame.
            /// @param nowMs Current time in ms.
            void drawStep(std::uint64_t nowMs);

            /// @brief Sets what the player's figure shows.
            /// @details Original: DrawScotdPlayerRole.
            /// @param nowMs Current time in ms.
            void drawPlayer(std::uint64_t nowMs);

            /// @brief Moves and sets what each NPC shows.
            /// @details Original: DrawScotdRoles.
            /// @param nowMs Current time in ms.
            void drawRoles(std::uint64_t nowMs);

            /// @brief Advances the hit flash.
            /// @details Original: DrawScotdCatchEffect.
            void drawEffect();

            /// @brief Updates the player's figure.
            /// @details Original: UpdateScotdPlayerRole.
            /// @param nowMs Current time in ms.
            void updatePlayer(std::uint64_t nowMs);

            /// @brief Updates the NPCs.
            /// @details Original: UpdateScotdRoles.
            /// @param nowMs Current time in ms.
            void updateRoles(std::uint64_t nowMs);

            /// @brief Costs a heart when an NPC runs into the player.
            /// @details Original: CheckScotdPlayerHit.
            void checkPlayerHit();

            /// @brief Marks NPCs hit by the player's swing.
            /// @details Original: CheckScotdRoleCatches.
            void checkCatches();

            /// @brief Steers the player towards the pressed point and swings when under it.
            /// @details Original: SteerScotdRoleToTargetX.
            /// @param nowMs Current time in ms.
            void steerToClick(std::uint64_t nowMs);

            /// @brief Makes a figure jump (right button, Up key).
            /// @param role The figure.
            void jump(ScotdRole& role) const;

            /// @brief Starts the player's swing (Space, or the mouse under the player).
            /// @param nowMs Current time in ms.
            void startSwing(std::uint64_t nowMs);

            /// @brief Drops a new NPC in somewhere on the field.
            /// @param nowMs Current time in ms.
            void spawn(std::uint64_t nowMs);

            /// @brief Steps a jump: rises then falls, choosing the frame of its height.
            /// @details Original: StepScotdRoleShift.
            /// @param role The figure.
            void stepShift(ScotdRole& role) const;

            /// @brief Ends the swing after its ticks.
            /// @details Original: StepScotdRoleFlag8522.
            void stepSwing();

            /// @brief Moves a figure sideways along the way it faces.
            /// @details Original: StepScotdRoleMoveX12.
            /// @param role The figure.
            void stepMoveX(ScotdRole& role) const;

            /// @brief Makes the screen point follow the position.
            /// @details Original: THuman.UpdateMove.
            /// @param role The figure.
            /// @param nowMs Current time in ms.
            void updateMove(ScotdRole& role, std::uint64_t nowMs) const;

            /// @brief Sets a figure's action and its direction slot.
            /// @details Original: SetHumanAction.
            /// @param role The figure.
            /// @param action The action.
            void setAction(ScotdRole& role, int action) const;

            /// @brief Ends the game and reports the result to the server.
            /// @param result Result code (0 lost, 1 won).
            void finish(int result);

            Hooks m_hooks;
            std::uint64_t m_lastFrameMs = 0;
            std::uint64_t m_nowMs = 0;
            bool m_started = false;
            bool m_over = false;
            int m_remaining = 0;
            int m_roleCount = 0;
            int m_tickCount = 0;
            int m_catchCount = 0;
            int m_endCounter = 0;
            core::Point m_click;
            bool m_mouseHeld = false;
            ScotdRole m_player;
            std::array<std::optional<ScotdRole>, kMaxRoles + 1> m_roles;
            bool m_effectOn = false;
            int m_effectFrame = 0;
            core::Point m_effectPoint;
            bool m_effectShown = false;
            core::Point m_effectAt;
    };

}

#endif
