#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_MOTION_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_FIGHTER_MOTION_H

#include "client/subsystems/battle/AfterImage.h"
#include "client/subsystems/battle/BattleCamera.h"
#include "core/Point.h"
#include "formats/MotionRecord.h"

#include <array>
#include <cstdint>
#include <vector>

namespace client {

    struct Fighter;

    /// @brief A fighter on the move during a skill: it runs or jumps to the motion's first point, steps from point to point, then goes back.
    /// @details Original: CH_TBattleMotion. Speed mode 0 runs or jumps from the cell to the first point, mode 1 steps from
    /// point to point (each with its pose, held frame and pace), mode 2 goes back. Positions are on the ground (the camera's
    /// coordinates); a motion the camera follows drags the camera along (FollowMotionWithCamera).
    class FighterMotion {
        public:
            /// @brief Resets the motion's timers and state.
            /// @details Original: ResetBattleMotion.
            /// @param nowMs Current time in milliseconds.
            void reset(std::uint64_t nowMs);

            /// @brief Loads the record's points from where the fighter stands, mirrored on side 1, moved by an offset.
            /// @details Original: SetupCasterMotion / SetupBattleMotion (startX / startY on the ground).
            /// @param record The motion record.
            /// @param side Side of the field the fighter is on.
            /// @param move Offset to move the points by.
            /// @param caster True for the caster's own motion.
            void load(const formats::MotionRecord& record, int side, core::Point move, bool caster);

            /// @brief Loads a status's motion at a cell, seen from the camera at rest.
            /// @details Original: SetupStatusMotion.
            /// @param record The motion record.
            /// @param side Side of the field the fighter is on.
            /// @param cell The fighter's cell.
            void loadStatus(const formats::MotionRecord& record, int side, core::Point cell);

            /// @brief Places the motion at a screen point.
            /// @details Original: PlaceMotionAt (SetMotionCameraPos first).
            /// @param screen Screen point.
            /// @param camera Camera position.
            void placeAt(core::Point screen, core::Point camera);

            /// @brief Sets the ground point the motion starts from.
            /// @param ground Ground point.
            void setStart(core::Point ground) { m_start = ground; }

            /// @brief Returns the ground point the motion starts from.
            /// @return The start point.
            core::Point start() const { return m_start; }

            static constexpr int kHoldSleepMs = 35; // Sleep(35) of the original, milliseconds

            /// @brief Returns the Sleep(35) AdvanceMotionPath makes while the fighter stays on a point of the path.
            /// @details Applies when the next point is the same place and the last point has been taken up (FPausedMs 0): every tick
            /// of such a stay, so a point whose time is short still takes a whole tick of 35 ms. AdvanceStatusMotionPath (a status
            /// motion's) has none.
            /// @return The sleep in milliseconds, 0 for none.
            int holdSleepMs() const;

            /// @brief Goes on along the points.
            /// @details Original: AdvanceMotionPath.
            /// @param camera The battle camera.
            /// @param fighterFollowed True when the fighter's skill has the camera.
            /// @param jumpHeight The fighter's current jump height.
            /// @param nowMs Current time in milliseconds.
            void advancePath(BattleCamera& camera, bool fighterFollowed, int jumpHeight, std::uint64_t nowMs);

            /// @brief Takes the next point once this one's time is up.
            /// @details Original: AdvanceMotionPoint.
            /// @param nowMs Current time in milliseconds.
            void advancePoint(std::uint64_t nowMs);

            /// @brief Works out where the fighter's feet are drawn.
            /// @details Original: CalcMotionDrawPos.
            void calcDrawPosition();

            /// @brief Runs or jumps the fighter to the first point.
            /// @details Original: StepMotionStage0.
            /// @param fighter The fighter.
            /// @param updateFighter True to update its pose too.
            void runToStart(Fighter& fighter, bool updateFighter) const;

            /// @brief Moves the fighter back to its cell.
            /// @details Original: StepMotionReturn.
            /// @param fighter The fighter.
            /// @param updateFighter True to update its pose too.
            /// @param camera The battle camera.
            /// @param nowMs Current time in milliseconds.
            void returnHome(Fighter& fighter, bool updateFighter, BattleCamera& camera, std::uint64_t nowMs);

            static constexpr int kAfterImages = 10; // most ghosts (screen effect 3 after-images) a fighter leaves, 1-based

            /// @brief Clears the ghosts and the frame timer (screen effect 3).
            /// @details Original: ClearAfterImages (FillChar of FAfterImages and the frame timer).
            /// @param nowMs Current time in milliseconds.
            void clearAfterImages(std::uint64_t nowMs);

            /// @brief Leaves a ghost of the fighter each time the frame is due (screen effect 3).
            /// @details Original: CalcMotionFrameDelay.
            /// @param fighter The fighter.
            /// @param nowMs Current time in milliseconds.
            void leaveAfterImage(const Fighter& fighter, std::uint64_t nowMs);

            /// @brief Returns the ghosts the fighter leaves on its run (1-based).
            /// @return The ghost array.
            std::array<AfterImage, kAfterImages + 1>& afterImages() { return m_afterImages; }

            /// @brief Returns where the camera was when the motion was last placed.
            /// @return The camera position.
            core::Point cameraPosition() const { return {m_cameraX, m_cameraY}; }

            /// @brief Returns the speed mode (0 run to the start, 1 along the points, 2 back).
            /// @return The speed mode.
            int speedMode() const { return m_speedMode; }

            /// @brief Sets the speed mode.
            /// @param mode The speed mode.
            void setSpeedMode(int mode) { m_speedMode = mode; }

            /// @brief Returns the index of the current path point.
            /// @return The point index.
            int pointIndex() const { return m_pointIndex; }

            /// @brief Returns the index of the last path point taken.
            /// @return The point index.
            int lastPointIndex() const { return m_lastPointIndex; }

            /// @brief Tells whether the path is done.
            /// @return True when done.
            bool pathDone() const { return m_pathDone; }

            /// @brief Tells whether the end of the path was reached.
            /// @return True when reached.
            bool pathEndReached() const { return m_pathEndReached; }

            /// @brief Tells whether the motion arrived at its target.
            /// @return True when arrived.
            bool arrived() const { return m_arrived; }

            /// @brief Clears the arrived flag.
            void clearArrived() { m_arrived = false; }

            /// @brief Tells whether the fighter is running.
            /// @return True when running.
            bool running() const { return m_running; }

            /// @brief Sets whether the fighter is running.
            /// @param running The new value.
            void setRunning(bool running) { m_running = running; }

            /// @brief Tells whether the camera follows this motion.
            /// @return True when following.
            bool cameraFollow() const { return m_cameraFollow; }

            /// @brief Sets whether the camera follows this motion.
            /// @param follow The new value.
            void setCameraFollow(bool follow) { m_cameraFollow = follow; }

            /// @brief Restarts the move timers.
            /// @param nowMs Current time in milliseconds.
            void restartMove(std::uint64_t nowMs) {
                m_moveStartMs = nowMs;
                m_moveStartMsY = nowMs;
            }

            /// @brief Returns the motion's position on the ground.
            /// @return The ground position.
            core::Point position() const { return {m_posX, m_posY}; }

            /// @brief Returns the motion's position on the screen.
            /// @return The screen position.
            core::Point screen() const { return {m_screenX, m_screenY}; }

            /// @brief Returns the fighter's home cell.
            /// @return The home point.
            core::Point home() const { return {m_homeX, m_homeY}; }

            /// @brief Returns the pose (sprite action) of a point.
            /// @param point Point index (1-based).
            /// @return The pose slot.
            int frameSlot(int point) const;

            /// @brief Returns the held frame of a point.
            /// @param point Point index (1-based).
            /// @return The held frame.
            int frameHold(int point) const;

            /// @brief Returns the point's slope (FCameraSlopes), the speed SetMoveSpeed gives the fighter.
            /// @param point Point index (1-based).
            /// @return The slope.
            double slope(int point) const;

        private:
            /// @brief Checks whether the motion arrived at its target.
            /// @details Original: CheckMotionArrived.
            void checkArrived();

            /// @brief Tells whether the motion moves on its own in x rather than with the camera.
            /// @details Original: IsMotionAtCameraEdgeX.
            /// @param camera The battle camera.
            /// @return True at the camera's x edge.
            bool atCameraEdgeX(const BattleCamera& camera);

            /// @brief Tells whether the motion moves on its own in y rather than with the camera.
            /// @details Original: IsMotionAtCameraEdgeY.
            /// @param camera The battle camera.
            /// @param jumpHeight The fighter's current jump height.
            /// @return True at the camera's y edge.
            bool atCameraEdgeY(const BattleCamera& camera, int jumpHeight);

            /// @brief Syncs the camera with the motion.
            /// @details Original: SyncCameraWithMotion.
            /// @param camera The battle camera.
            void syncCamera(BattleCamera& camera);

            /// @brief Drags the camera along with the motion.
            /// @details Original: FollowMotionWithCamera.
            /// @param camera The battle camera.
            /// @param jumpHeight The fighter's current jump height.
            /// @param nowMs Current time in milliseconds.
            void followWithCamera(BattleCamera& camera, int jumpHeight, std::uint64_t nowMs);

            /// @brief Moves the motion on its own, without the camera.
            /// @details Original: StepMotion.
            /// @param camera The battle camera.
            /// @param nowMs Current time in milliseconds.
            void stepAlone(const BattleCamera& camera, std::uint64_t nowMs);

            /// @brief Steps a coordinate toward its target in x.
            /// @details Original: StepMotionTowardX.
            /// @param position Current x.
            /// @param target Target x.
            /// @param nowMs Current time in milliseconds.
            /// @return The new x.
            int stepTowardX(int position, int target, std::uint64_t nowMs);

            /// @brief Steps a coordinate toward its target in y.
            /// @details Original: StepMotionTowardY.
            /// @param position Current y.
            /// @param target Target y.
            /// @param nowMs Current time in milliseconds.
            /// @return The new y.
            int stepTowardY(int position, int target, std::uint64_t nowMs);

            /// @brief Converts elapsed time into a step length at the current slope.
            /// @param elapsed Elapsed time in milliseconds.
            /// @return The step in pixels.
            int speedStep(double elapsed) const;

            /// @brief Begins a segment of the path.
            /// @details Original: BeginMotionSegment.
            /// @param from Start of the segment.
            /// @param to End of the segment.
            void beginSegment(core::Point from, core::Point to);

            /// @brief Converts a screen position to the motion position.
            /// @details Original: ScreenToMotionPos.
            /// @param x Screen x.
            /// @param y Screen y.
            void screenToPosition(int x, int y);

            /// @brief Sets the anchored motion position.
            /// @details Original: SetMotionAnchoredPos.
            /// @param x Anchored x.
            /// @param y Anchored y.
            void anchor(int x, int y);

            /// @brief Lifts the fighter along the jump arc.
            /// @details Original: UpdateFighterJump.
            /// @param fighter The fighter.
            /// @param scale Height scale of the arc.
            void jumpArc(Fighter& fighter, float scale) const;

            /// @brief Returns the pose for running toward a point (64 / 65, or the NPC kind's own).
            /// @param fighter The fighter.
            /// @param towardRight True when running to the right.
            /// @return The pose slot.
            static int runPose(const Fighter& fighter, bool towardRight);

            /// @brief Returns the pose for jumping toward a point (36 / 37, or the NPC kind's own).
            /// @param fighter The fighter.
            /// @return The pose slot.
            static int jumpPose(const Fighter& fighter);

            int m_screenX = 0;
            int m_screenY = 0;
            bool m_arrived = false;
            int m_horzAnchor = 0;
            int m_anchorMode = 0;
            std::vector<int> m_pointDurations;
            std::vector<double> m_slopes;
            std::vector<int> m_frameSlots;
            std::vector<int> m_frameHolds;
            std::uint64_t m_moveStartMs = 0;
            std::uint64_t m_moveStartMsY = 0;
            std::uint64_t m_pathStepMs = 0;
            int m_posX = 0;
            int m_posY = 0;
            int m_cameraX = 0;
            int m_cameraY = 0;
            int m_targetX = 0;
            int m_targetY = 0;
            int m_segDX = 0;
            int m_segDY = 0;
            int m_pathPointCount = 0;
            int m_pathIndex = 0;
            int m_lastPointIndex = 0;
            int m_pointIndex = 0;
            int m_segLength = 0;
            bool m_segStarted = false;
            bool m_pathDone = false;
            bool m_pathEndReached = false;
            std::vector<core::Point> m_pathPoints;
            bool m_pathAdvanced = false;
            int m_moveKind1 = 0;
            int m_moveKind2 = 0;
            int m_speedMode = 0;
            float m_jumpScale1 = 0;
            float m_jumpScale2 = 0;
            bool m_cameraFollow = false;
            core::Point m_start;
            bool m_running = false;
            int m_drawPosMode = 0;
            std::vector<core::Point> m_groundPoints;
            int m_groundPointIndex = 0;
            std::vector<std::pair<int, int>> m_groundSegments;
            int m_groundSegIndex = 1;
            int m_homeX = 0;
            int m_homeY = 0;
            bool m_timedStepOn = false;
            double m_pausedMs = 0;
            double m_lastElapsed = 0;
            std::array<AfterImage, kAfterImages + 1> m_afterImages{};
            int m_frameDelay = 0;
            bool m_frameTimed = false;
            std::uint64_t m_frameTickMs = 0;
    };

}

#endif
