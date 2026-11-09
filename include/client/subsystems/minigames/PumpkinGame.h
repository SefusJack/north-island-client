#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_PUMPKIN_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_PUMPKIN_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/HumanFrame.h"
#include "client/subsystems/minigames/PumpkinGhost.h"
#include "core/Point.h"
#include "core/Size.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The pumpkin minigame (sport kind 17): aim an arrow, throw pumpkins at rising ghosts.
    /// @details Original: LD_MiniGame.pas TPumpkin; update runs twice per frame (30 ms); 30 hits in 50 s win, 3 hearts.
    class PumpkinGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kGhosts = 10; // ghosts in play, 1-based
            static constexpr core::Point kPlayerAt{400, 309}; // where the player stands: x, y in px

            /// @brief Game phase.
            /// @details Original: FState (2 countdown to the start, 3 playing, 0 over).
            enum class Phase { Over = 0, Countdown = 2, Playing = 3 };

            /// @brief Callbacks the game needs from its host (instructions, picture sizes, frame counts, random).
            struct Hooks : GameHooks {
                std::function<bool()> instructionsOpen;
                std::function<core::Size(const std::string&)> pictureSize;
                std::function<void(core::Point)> hitLight;
                std::function<int(int slot)> playerFrames;
                std::function<int(int slot)> ghostFrames;
                std::function<int(int count)> random;
            };

            /// @brief Creates the game.
            /// @param hooks Host callbacks.
            explicit PumpkinGame(
                Hooks hooks
            );

            /// @brief Starts a game (resets state).
            /// @details Original: TPumpkin.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Leaves the game.
            /// @details Original: ExitPumpkinGame (the Quit button, the instructions' Leave).
            void exitGame();

            /// @brief Instructions' Begin button: shows the game form.
            /// @details Original: PumpkinForm.ShowCtrl.
            void begin() { m_formShown = true; }

            /// @brief Throws a pumpkin along the arrow.
            /// @details Original: PumpkinMouseDown.
            void mouseDown();

            /// @brief Runs the frames that have passed.
            /// @details Original: UpdatePumpkinGame twice and DrawPumpkinGame per frame.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

            /// @brief Whether the game form is shown.
            /// @return True once Begin was pressed.
            bool formShown() const { return m_formShown; }

            /// @brief Current phase.
            /// @return The phase.
            Phase phase() const { return m_phase; }

            /// @brief Ghost hits so far.
            /// @return The score.
            int score() const { return m_score; }

            /// @brief Hearts left.
            /// @return The heart count.
            int hearts() const { return m_hearts; }

            /// @brief Seconds left.
            /// @return The remaining time.
            int timeLeft() const { return m_timeLeft; }

            /// @brief Start countdown value.
            /// @return The countdown.
            int startCountdown() const { return m_startCountdown; }

            /// @brief All ghosts (index 0 unused).
            /// @return The ghost array.
            const std::array<PumpkinGhost, kGhosts + 1>& ghosts() const { return m_ghosts; }

            /// @brief Frame of the player's figure.
            /// @return The frame index.
            int playerFrame() const { return m_playerFrame; }

            /// @brief Whether the pumpkin in flight was drawn by the last update.
            /// @return True if shown.
            bool bombShown() const { return m_bombShown; }

            /// @brief Centre of the pumpkin in flight as last drawn.
            /// @return The position.
            core::Point bombAt() const { return m_bombShownAt; }

            /// @brief Tip of the aim arrow.
            /// @details Original: DrawPumpkinAimLine.
            /// @return The tip position.
            core::Point aimTip() const { return m_aim; }

            /// @brief First barb of the aim arrow.
            /// @return The barb position.
            core::Point aimBarbA() const { return m_arrowA; }

            /// @brief Second barb of the aim arrow.
            /// @return The barb position.
            core::Point aimBarbB() const { return m_arrowB; }

            /// @brief Words shown over the screen at the end.
            /// @return The end text.
            std::string overText() const;

            /// @brief Fraction of the way from the last frame to the next.
            /// @param nowMs Current time in ms.
            /// @return 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Whether the pumpkin was shown in the frame before.
            /// @return True if shown.
            bool previousBombShown() const { return m_previousBombShown; }

            /// @brief Pumpkin position in the frame before.
            /// @return The position.
            core::Point previousBombAt() const { return m_previousBombAt; }

            /// @brief Arrow tip in the frame before.
            /// @return The position.
            core::Point previousAimTip() const { return m_previousAim; }

            /// @brief First arrow barb in the frame before.
            /// @return The position.
            core::Point previousAimBarbA() const { return m_previousArrowA; }

            /// @brief Second arrow barb in the frame before.
            /// @return The position.
            core::Point previousAimBarbB() const { return m_previousArrowB; }

        private:
            /// @brief The thrown pumpkin (objManage.FBombs[1]).
            struct Bomb {
                core::Point pos{1, 1};
                bool flying = false;
                bool hit = false;
            };

            /// @brief Runs one logic frame.
            /// @param nowMs Current time in ms.
            void updateOnce(std::uint64_t nowMs);

            /// @brief Advances the start countdown.
            /// @param nowMs Current time in ms.
            void countdownStep(std::uint64_t nowMs);

            /// @brief Ticks the game clock (seconds left).
            /// @param nowMs Current time in ms.
            void tickClock(std::uint64_t nowMs);

            /// @brief Updates the playing state.
            /// @param nowMs Current time in ms.
            void updatePlay(std::uint64_t nowMs);

            /// @brief Swings the aim arrow.
            void updateAim();

            /// @brief Moves the pumpkin in flight.
            void updateShot();

            /// @brief Updates one ghost.
            /// @param index Ghost index.
            /// @param nowMs Current time in ms.
            void updateGhost(int index, std::uint64_t nowMs);

            /// @brief Tests whether the pumpkin hits a ghost.
            /// @param ghost The ghost.
            /// @return True on a hit.
            bool shotHits(const PumpkinGhost& ghost) const;

            /// @brief Ends the game and reports the result to the server.
            /// @param result Result code.
            void finish(int result);

            Hooks m_hooks;
            std::uint64_t m_lastFrameMs = 0;
            bool m_formShown = false;
            Phase m_phase = Phase::Countdown;
            int m_score = 0;
            int m_hearts = 3; // hearts (lives) at the start
            int m_timeLeft = 50; // seconds on the clock at the start
            int m_startCountdown = 4; // start countdown, seconds
            std::uint64_t m_countdownTickMs = 0;
            std::uint64_t m_endTickMs = 0;
            std::array<PumpkinGhost, kGhosts + 1> m_ghosts;
            Bomb m_bomb;
            bool m_bombShown = false;
            core::Point m_bombShownAt;
            bool m_previousBombShown = false;
            core::Point m_previousBombAt;
            HumanFrame m_playerAnim;
            int m_playerFrame = 0;
            int m_tick = 0;
            double m_angle = 0;
            bool m_reverse = false;
            core::Point m_aim;
            core::Point m_arrowA;
            core::Point m_arrowB;
            core::Point m_previousAim;
            core::Point m_previousArrowA;
            core::Point m_previousArrowB;
            core::Point m_shotFrom;
            double m_slope = 0;
            double m_slopeSum = 0;
            int m_shotDx = 0;
            int m_shotDy = 0;
    };

}

#endif
