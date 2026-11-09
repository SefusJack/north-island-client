#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_CAMERA_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_CAMERA_H

#include "core/Point.h"
#include "formats/MotionCameraStep.h"

#include <array>
#include <cstdint>
#include <vector>

namespace client {

    /// @brief The battle camera: the top left of the 800 x 600 view on the 1664 x 832 ground, its resting place, the far layer's place and skill motion paths.
    /// @details Original: CH_TBattleGround's camera. The far layer moves at the camera's speed over 1.5.
    class BattleCamera {
        public:
            static constexpr int kViewWidth = 800; // view width, px
            static constexpr int kViewHeight = 600; // view height, px
            static constexpr int kHalfWidth = kViewWidth / 2;
            static constexpr int kHalfHeight = kViewHeight / 2;
            static constexpr core::Point kRest{432, 232}; // camera position at rest on the ground, px

            /// @brief Puts the camera back at rest.
            /// @details Original: ResetBattleLayers.
            /// @param nowMs Current time in ms.
            void reset(std::uint64_t nowMs);

            /// @brief Clears the camera path.
            /// @details Original: ResetCameraPath.
            /// @param nowMs Current time in ms.
            void resetPath(std::uint64_t nowMs);

            /// @brief The top left of the view on the ground.
            /// @return View position.
            core::Point position() const { return {m_x, m_y}; }

            /// @brief The far layer's own x place.
            /// @return Far layer x.
            int farX() const { return m_farX; }

            /// @brief Whether the camera is at its resting place.
            /// @return True at rest.
            bool atRest() const { return m_x == kRest.x && m_y == kRest.y; }

            /// @brief Sets the size of the ground's near layer (the edges the camera keeps within).
            /// @param width Ground width.
            /// @param height Ground height.
            void setGroundSize(int width, int height);

            /// @brief The ground width.
            /// @return Width in pixels.
            int groundWidth() const { return m_groundWidth; }

            /// @brief The ground height.
            /// @return Height in pixels.
            int groundHeight() const { return m_groundHeight; }

            /// @brief Puts the view at a place, kept on the ground.
            /// @details Original: SetCameraPos.
            /// @param x View left.
            /// @param y View top.
            void moveTo(int x, int y);

            /// @brief Puts the view and its far layer as given, for a drawing between two ticks (BattleTween).
            /// @param position View position.
            /// @param farX Far layer x.
            void showAt(core::Point position, int farX) {
                m_x = position.x;
                m_y = position.y;
                m_farX = farX;
            }

            /// @brief Begins a move unless one is under way.
            /// @details Original: StartCameraMove.
            /// @param from Start place.
            /// @param to Target place.
            void startMove(core::Point from, core::Point to);

            /// @brief Does a step of the move.
            /// @details Original: MoveCamera; speed 0 takes the path step's own.
            /// @param speed Step speed.
            /// @param nowMs Current time in ms.
            void step(double speed, std::uint64_t nowMs);

            /// @brief Whether the camera is at its move's target.
            /// @details Original: IsCameraAtTarget.
            /// @return True at the target.
            bool atTarget() const;

            /// @brief Stops the move under way.
            void stopMoving() { m_moving = false; }

            /// @brief Loads a skill's camera path; side 1 mirrors it.
            /// @details Original: LoadCameraPath.
            /// @param steps The path steps.
            /// @param side Battle side.
            void loadPath(const std::vector<formats::MotionCameraStep>& steps, int side);

            /// @brief Follows the camera path at a step of the motion.
            /// @details Original: FollowCameraPath.
            /// @param motionPoint Current step of the motion.
            /// @param nowMs Current time in ms.
            void followPath(int motionPoint, std::uint64_t nowMs);

            /// @brief Does a jumping path step at a step of the motion.
            /// @details Original: JumpCameraStep.
            /// @param centre Middle to jump to.
            /// @param motionPoint Current step of the motion.
            /// @param nowMs Current time in ms.
            void jumpStep(core::Point centre, int motionPoint, std::uint64_t nowMs);

            /// @brief The number of path steps.
            /// @return Step count.
            int pathSteps() const { return static_cast<int>(m_path.size()); }

            /// @brief The focus kind of the path's current step (or the one before).
            /// @param step Step index.
            /// @return Focus kind, 0 none.
            int focus(int step) const;

            /// @brief Makes the last step go back to the resting place.
            void endPathAtRest();

            /// @brief Sets the follow parameters.
            /// @details Original: CalcCameraStepX / CalcCameraStepY inputs.
            /// @param slope Slope of the followed motion.
            /// @param distX Horizontal distance.
            /// @param distY Vertical distance.
            void setFollow(double slope, int distX, int distY) {
                m_slope = slope;
                m_distX = distX;
                m_distY = distY;
            }

            /// @brief Where a followed motion puts the view's middle, horizontally.
            /// @details Original: CalcCameraStepX.
            /// @param current Current x.
            /// @param target Target x.
            /// @param startMs Start time of the move.
            /// @param pausedMs Paused time.
            /// @param speedMode Speed mode.
            /// @param moveKind Move kind.
            /// @param nowMs Current time in ms.
            /// @return The next x.
            int followStepX(
                int current,
                int target,
                std::uint64_t startMs,
                double pausedMs,
                int speedMode,
                int moveKind,
                std::uint64_t nowMs
            );

            /// @brief Where a followed motion puts the view's middle, vertically.
            /// @details Original: CalcCameraStepY.
            /// @param current Current y.
            /// @param target Target y.
            /// @param startMs Start time of the move.
            /// @param pausedMs Paused time.
            /// @param speedMode Speed mode.
            /// @param moveKind Move kind.
            /// @param nowMs Current time in ms.
            /// @return The next y.
            int followStepY(
                int current,
                int target,
                std::uint64_t startMs,
                double pausedMs,
                int speedMode,
                int moveKind,
                std::uint64_t nowMs
            ) const;

            /// @brief Switches the skill camera: a step without its own speed heads back to rest.
            /// @details Original: FSkillCameraOn.
            /// @param on Whether on.
            void setSkillCamera(bool on) { m_skillCamera = on; }

            /// @brief Restarts the move and step clocks.
            /// @param nowMs Current time in ms.
            void restartTicks(std::uint64_t nowMs) {
                m_moveMs = nowMs;
                m_stepMs = nowMs;
            }

        private:
            /// @brief Whether a path step is done.
            /// @details Original: IsCameraStepDone; the step's end is a motion step below 101 or milliseconds.
            /// @param until The step's end value.
            /// @param motionPoint Current step of the motion.
            /// @param nowMs Current time in ms.
            /// @return True when done.
            bool stepDone(int until, int motionPoint, std::uint64_t nowMs) const;

            int m_x = kRest.x;
            int m_y = kRest.y;
            int m_farX = kRest.x;
            // default ground size in pixels (1664 x 832)
            int m_groundWidth = 1664;
            int m_groundHeight = 832;
            bool m_moving = false;
            core::Point m_from;
            core::Point m_to;
            int m_distX = 0;
            int m_distY = 0;
            double m_slope = 0;
            std::uint64_t m_moveMs = 0;
            std::uint64_t m_stepMs = 0;
            bool m_skillCamera = false;

            /// @brief One step of a camera path.
            struct PathStep {
                core::Point point;
                double speed = 0;
                int until = 0;
                int focus = 0;
            };

            std::vector<PathStep> m_path;
            int m_pathStep = 1;
            bool m_stepArrived = false;
            bool m_stepMoving = true;
            std::uint64_t m_pathMs = 0;
            bool m_jumpReady = false;
            bool m_jumpWaiting = false;
    };

}

#endif
