#include "client/subsystems/minigames/ClawCatcher.h"

namespace client {

    namespace {

        constexpr int kMovingFrames = 4; // frames of the claw coming down
        constexpr int kMovingDelay = 1; // game frames each moving frame is shown
        constexpr int kCatchingFrames = 5; // frames of the claw closing
        constexpr int kCatchingDelay = 4; // game frames each closing frame is shown
        constexpr int kCatchingSlowFrame = 2; // closing frame after which the claw pauses
        constexpr int kCatchingSlowDelay = 18; // game frames shown for that slow closing frame
        constexpr int kDroppingFrames = 4; // frames of the doll dropping into the hole
        constexpr int kDroppingDelay = 10; // game frames each dropping frame is shown at first
        constexpr int kDroppingSlowFrame = 1; // dropping frame that is shown longer
        constexpr int kDroppingSlowDelay = 15; // game frames shown for that slow dropping frame
        constexpr int kDroppingDelayAfter = 4; // game frames shown for the dropping frames after the slow one
        constexpr int kHoldFrames = 20; // game frames the claw waits at the bottom and at the top

    }

    ClawCatcher::ClawCatcher() {
        m_moving.count = kMovingFrames;
        m_moving.delay = kMovingDelay;
        m_catching.count = kCatchingFrames;
        m_catching.delay = kCatchingDelay;
        m_droppingPicture.count = kDroppingFrames;
        m_droppingPicture.delay = kDroppingDelay;
    }

    void ClawCatcher::reset() {
        m_moving.reset();
        m_catching.reset();
        m_droppingPicture.reset();
        m_x = kHomeX;
        m_y = kHomeY;
        m_state = State::Idle;
        m_delayCount = 0;
        m_returning = false;
        m_grabbed = false;
        m_risen = false;
        m_result = 0;
        m_lastLiftFrame = -1;
    }

    void ClawCatcher::clearGrab() {
        m_closed = false;
        m_dropsIntoHole = false;
        m_grabbed = false;
        m_fallsBack = false;
        m_doll = -1;
        m_result = 0;
        m_heldY = 0;
    }

    void ClawCatcher::setMoveState(State state) {
        if (m_state != State::Lifting && m_state != State::Grasping && m_state != State::Returning) {
            m_state = state;
        }
    }

    void ClawCatcher::moveX() {
        switch (m_state) {
            case State::Left: // moving left along the rail
            case State::Returning: // returning to the hole (left end)
                if (m_x - kStep >= kHomeX) {
                    m_x -= kStep;
                }
                break;
            case State::Right: // moving right along the rail
                if (m_x + kStep <= kMostRightX) {
                    m_x += kStep;
                }
                break;
            default: // idle, grasping, lifting: no rail movement
                break;
        }
    }

    bool ClawCatcher::delayElapsed(int threshold) {
        ++m_delayCount;
        if (m_delayCount >= threshold) {
            m_delayCount = 0;
            return true;
        }
        return false;
    }

    void ClawCatcher::advanceMoving() { m_blits.push_back({Picture::Moving, m_moving.advance()}); }

    void ClawCatcher::advanceCatching() {
        const int before = m_catching.frame;
        const int shown = m_catching.advance();
        if (m_catching.frame != before) {
            m_catching.delay = m_catching.frame == kCatchingSlowFrame ? kCatchingSlowDelay : kCatchingDelay;
        }
        m_blits.push_back({Picture::Catching, shown});
    }

    void ClawCatcher::stepBackMoving() { m_blits.push_back({Picture::Moving, m_moving.stepBack()}); }

    void ClawCatcher::draw() {
        m_blits.clear();
        switch (m_state) {
            case State::Idle: // resting claw picture
            case State::Left: // moving left: same resting picture
            case State::Right: // moving right: same resting picture
                m_blits.push_back({Picture::Basic, 0});
                break;
            case State::Grasping: // coming down and closing, then the hold delay
                if (!m_moving.isLast() && !m_catching.isLast()) {
                    advanceMoving();
                }
                if (m_moving.isLast() && !m_catching.isLast()) {
                    if (m_catching.frame + 1 == m_catching.count) {
                        m_closed = true;
                    }
                    advanceCatching();
                }
                if (!m_returning && m_moving.isLast() && m_catching.isLast()) {
                    if (!delayElapsed(kHoldFrames)) {
                        advanceCatching();
                    } else {
                        advanceCatching();
                        m_returning = true;
                        m_state = State::Lifting;
                    }
                }
                if (m_returning && m_catching.isLast()) {
                    stepBackMoving();
                    if (m_moving.isFirst() && m_catching.isLast()) {
                        if (!delayElapsed(kHoldFrames)) {
                            stepBackMoving();
                            return;
                        }
                        m_state = State::Returning;
                    }
                }
                break;
            case State::Lifting: // going back up with the doll
                if (m_returning && m_catching.isLast()) {
                    stepBackMoving();
                    if (m_moving.isFirst() && m_catching.isLast()) {
                        m_state = State::Grasping;
                        m_risen = true;
                    }
                }
                break;
            case State::Returning: // carrying the doll to the hole, then the dropping picture
                if (m_x > kHomeX) {
                    m_blits.push_back({Picture::Basic, 0});
                }
                if (m_x <= kHomeX) {
                    const int before = m_droppingPicture.frame;
                    const int shown = m_droppingPicture.advance();
                    if (m_droppingPicture.frame != before) {
                        m_droppingPicture.delay = m_droppingPicture.frame == kDroppingSlowFrame ? kDroppingSlowDelay : kDroppingDelayAfter;
                    }
                    m_blits.push_back({Picture::Dropping, shown});
                }
                break;
        }
    }

}
