#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_CLAW_CATCHER_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_CLAW_CATCHER_H

#include "client/subsystems/minigames/FramePicture.h"

#include <vector>

namespace client {

    /// @brief The doll machine claw (original MZ_Catcher.pas TCatcher): rail movement, grasp/lift/return states and the blits to draw.
    class ClawCatcher {
        public:
            static constexpr int kHomeX = 178; // claw's rest x on the machine, px (left end of the rail)
            static constexpr int kHomeY = 108; // claw's y on the machine, px
            static constexpr int kMostRightX = 484; // rightmost x the claw may reach, px
            static constexpr int kStep = 3; // px the claw moves per frame
            static constexpr int kClawWidth = 154; // width of a claw picture frame, px
            static constexpr int kBasicHeight = 126; // height of one claw frame at rest, px
            static constexpr int kMovingHeight = 235; // height of one claw frame while moving, px
            static constexpr int kCatchingHeight = 242; // height of one claw frame while grasping, px
            static constexpr int kDroppingHeight = 128; // height of one claw frame while dropping, px

            /// @brief Claw movement state (original FObjects.V[2]).
            enum class State { Idle = 0, Left = 1, Right = 2, Grasping = 3, Returning = 4, Lifting = 5 };

            /// @brief The four claw pictures (original CatcherBasic, CatcherMoving, Catching, CatcherDroping).
            enum class Picture { Basic, Moving, Catching, Dropping };

            /// @brief One picture and frame to draw for the claw.
            struct Blit {
                Picture picture = Picture::Basic;
                int frame = 0;
            };

            /// @brief Creates the claw at its home place with the three frame pictures set up (original TCatcher.Create).
            ClawCatcher();

            /// @brief Resets the pictures' frames and the claw's place and state (original ResetCatcher; what is held is kept).
            void reset();

            /// @brief Lets go of whatever is held (original ClearCatcherGrab).
            void clearGrab();

            /// @brief Sets the movement state unless the claw is grasping, lifting or returning (original TCatcher.SetCatcherMoveState).
            /// @param state New state.
            void setMoveState(State state);

            /// @brief Moves the claw one step along the rail for the current state (original MoveCatcherX).
            void moveX();

            /// @brief Builds this frame's blit list and advances the grasp/lift/return animation (original DrawCatcherClaw).
            void draw();

            /// @brief Tells whether the claw is free to move (original IsCatcherIdle).
            /// @return True unless grasping, lifting or returning.
            bool isIdle() const { return m_state != State::Lifting && m_state != State::Grasping && m_state != State::Returning; }

            /// @brief Left edge of the catching area (original GetClawLeftX).
            /// @return X coordinate.
            int clawLeftX() const { return m_x + 20; } // left inset of the catching area from the claw's x

            /// @brief Right edge of the catching area.
            /// @return X coordinate.
            int clawRightX() const { return kClawWidth + m_x - 20; } // right inset of the catching area

            /// @brief Left edge of the claw's centre zone.
            /// @return X coordinate.
            int clawCenterLeftX() const { return m_x + 60; } // centre zone left offset from the claw's x

            /// @brief Right edge of the claw's centre zone.
            /// @return X coordinate.
            int clawCenterRightX() const { return m_x + 75; } // centre zone right offset from the claw's x

            /// @brief Claw x position.
            /// @return X coordinate.
            int x() const { return m_x; }

            /// @brief Claw y position.
            /// @return Y coordinate.
            int y() const { return m_y; }

            /// @brief Current movement state.
            /// @return The state.
            State state() const { return m_state; }

            /// @brief Pictures to draw this frame.
            /// @return The blit list built by draw().
            const std::vector<Blit>& blits() const { return m_blits; }

            /// @brief Held result (original FObjects.V[6]).
            /// @return 0 nothing, 1 a doll that will fall again, 2 a doll held in the middle.
            int result() const { return m_result; }

            /// @brief Sets the held result (FObjects.V[6]).
            /// @param result New result value.
            void setResult(int result) { m_result = result; }

            /// @brief Index of the doll held (original FObjects.V[7]).
            /// @return Doll index, -1 for none.
            int doll() const { return m_doll; }

            /// @brief Sets the held doll (FObjects.V[7]).
            /// @param doll Doll index, -1 for none.
            void setDoll(int doll) { m_doll = doll; }

            /// @brief Whether something is held (original FObjects.B[20]).
            /// @return True when held.
            bool grabbed() const { return m_grabbed; }

            /// @brief Sets the held flag (FObjects.B[20]).
            /// @param grabbed New value.
            void setGrabbed(bool grabbed) { m_grabbed = grabbed; }

            /// @brief Whether the claw has closed on the doll (original FObjects.B[33]).
            /// @return True when closed.
            bool closed() const { return m_closed; }

            /// @brief Sets the closed flag (FObjects.B[33]).
            /// @param closed New value.
            void setClosed(bool closed) { m_closed = closed; }

            /// @brief Whether the claw is up with the doll (original FObjects.B[34]).
            /// @return True when risen.
            bool risen() const { return m_risen; }

            /// @brief Sets the risen flag (FObjects.B[34]).
            /// @param risen New value.
            void setRisen(bool risen) { m_risen = risen; }

            /// @brief Whether the badly held doll falls back (original FObjects.B[35]).
            /// @return True when it falls back.
            bool fallsBack() const { return m_fallsBack; }

            /// @brief Sets the falls-back flag (FObjects.B[35]).
            /// @param fallsBack New value.
            void setFallsBack(bool fallsBack) { m_fallsBack = fallsBack; }

            /// @brief Whether the doll drops through the hole (original FObjects.B[32]).
            /// @return True when dropping.
            bool dropsIntoHole() const { return m_dropsIntoHole; }

            /// @brief Sets the drops-into-hole flag (FObjects.B[32]).
            /// @param drops New value.
            void setDropsIntoHole(bool drops) { m_dropsIntoHole = drops; }

            /// @brief Moving-picture frame the lift last followed (original FObjects.V[9]).
            /// @return Frame, -1 for none.
            int lastLiftFrame() const { return m_lastLiftFrame; }

            /// @brief Sets the last lift frame (FObjects.V[9]).
            /// @param frame Frame, -1 for none.
            void setLastLiftFrame(int frame) { m_lastLiftFrame = frame; }

            /// @brief Y where the doll stood when first caught (original FObjects.V[11]).
            /// @return Y coordinate.
            int heldY() const { return m_heldY; }

            /// @brief Sets the held doll's original y (FObjects.V[11]).
            /// @param y Y coordinate.
            void setHeldY(int y) { m_heldY = y; }

            /// @brief The moving picture (original V[14]).
            /// @return Mutable reference.
            FramePicture& moving() { return m_moving; }

            /// @brief The catching picture (original V[15]).
            /// @return Mutable reference.
            FramePicture& catching() { return m_catching; }

            /// @brief The dropping picture (original V[16]).
            /// @return Mutable reference.
            FramePicture& droppingPicture() { return m_droppingPicture; }

            /// @brief The moving picture (original V[14]).
            /// @return Const reference.
            const FramePicture& moving() const { return m_moving; }

            /// @brief The catching picture (original V[15]).
            /// @return Const reference.
            const FramePicture& catching() const { return m_catching; }

            /// @brief The dropping picture (original V[16]).
            /// @return Const reference.
            const FramePicture& droppingPicture() const { return m_droppingPicture; }

        private:
            /// @brief Counts one frame and tells whether the delay has run out (original CatcherDelayElapsed).
            /// @param threshold Frames to wait.
            /// @return True when the threshold was reached (counter reset).
            bool delayElapsed(int threshold);

            /// @brief Advances the moving picture and adds its blit (original TPicMoving.DrawPicAdvance).
            void advanceMoving();

            /// @brief Advances the catching picture, adds its blit and sets the delay to the next frame (original TPicCatching.DrawPicAdvance).
            void advanceCatching();

            /// @brief Steps the moving picture back and adds its blit (original DrawPicStepBack).
            void stepBackMoving();

            int m_x = kHomeX;
            int m_y = kHomeY;
            State m_state = State::Idle;
            int m_delayCount = 0;
            bool m_returning = false;
            int m_result = 0;
            int m_doll = -1;
            int m_lastLiftFrame = -1;
            int m_heldY = 0;
            bool m_grabbed = false;
            bool m_dropsIntoHole = false;
            bool m_closed = false;
            bool m_risen = false;
            bool m_fallsBack = false;
            FramePicture m_moving;
            FramePicture m_catching;
            FramePicture m_droppingPicture;
            std::vector<Blit> m_blits;
    };

}

#endif
