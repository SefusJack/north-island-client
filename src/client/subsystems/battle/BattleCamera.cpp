#include "client/subsystems/battle/BattleCamera.h"

#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kGroundWidth = 1664; // battle ground width, px (two 832 px screens wide)
        constexpr int kGroundHeight = 832; // battle ground height, px
        constexpr double kNoZero = 0.0001; // added to a divisor so it is never zero

        /// @brief Rounds like Delphi's Round: halves to even.
        /// @param value Value to round.
        /// @return The rounded integer.
        int round(double value) { return static_cast<int>(std::lrint(value)); }

    }

    void BattleCamera::reset(std::uint64_t nowMs) {
        moveTo(kRest.x, kRest.y);
        m_farX = kRest.x;
        resetPath(nowMs);
    }

    void BattleCamera::resetPath(std::uint64_t nowMs) {
        m_pathStep = 1;
        m_stepArrived = false;
        m_stepMoving = true;
        m_pathMs = nowMs;
        m_moveMs = nowMs;
        m_stepMs = nowMs;
        m_moving = false;
        m_skillCamera = false;
    }

    void BattleCamera::setGroundSize(int width, int height) {
        m_groundWidth = width;
        m_groundHeight = height;
    }

    void BattleCamera::moveTo(int x, int y) {
        m_x = x < 0 ? 0 : (x + kViewWidth > kGroundWidth ? kGroundWidth - kViewWidth : x);
        m_y = y < 0 ? 0 : (y + kViewHeight > kGroundHeight ? kGroundHeight - kViewHeight : y);
    }

    void BattleCamera::startMove(core::Point from, core::Point to) {
        if (m_moving) {
            return;
        }
        m_distX = std::abs(from.x - to.x);
        m_distY = std::abs(from.y - to.y);
        m_from = from;
        m_to = to;
        m_moving = true;
    }

    bool BattleCamera::atTarget() const {
        if (m_distX >= m_distY) {
            return std::abs(m_x - m_to.x) <= 0;
        }
        return std::abs(m_y - m_to.y) <= 0;
    }

    void BattleCamera::step(double speed, std::uint64_t nowMs) {
        double elapsed = static_cast<double>(nowMs - m_moveMs);
        m_moveMs = nowMs;
        const double pathSpeed = m_pathStep >= 1 && m_pathStep <= pathSteps() ? m_path[static_cast<std::size_t>(m_pathStep - 1)].speed : 0;
        int stepX = 0;
        int stepFar = 0;
        if (speed == 0) {
            if (m_skillCamera) {
                m_to = kRest;
                m_skillCamera = false;
            }
            stepX = round(elapsed * pathSpeed);
            stepFar = round(pathSpeed / 1.5 * elapsed); // the far layer moves at the camera's speed over 1.5
        } else {
            stepX = round(elapsed * speed);
            stepFar = round(speed / 1.5 * elapsed); // the far layer moves at the camera's speed over 1.5
        }
        int x = m_x;
        const int savedFar = m_farX;
        const double ratioX = m_distX / (m_distY + kNoZero);
        if (m_distX >= m_distY) {
            if (m_from.x < m_to.x) {
                m_farX += stepFar;
                x += stepX;
                if (x >= m_to.x) {
                    x = m_to.x;
                    m_farX = savedFar;
                }
            } else if (m_from.x > m_to.x) {
                m_farX -= stepFar;
                x -= stepX;
                if (x <= m_to.x) {
                    x = m_to.x;
                    m_farX = savedFar;
                }
            }
        } else {
            if (m_from.x < m_to.x) {
                m_farX += round(ratioX * stepFar);
                x += round(ratioX * stepX);
                if (x >= m_to.x) {
                    x = m_to.x;
                    m_farX = savedFar;
                }
            } else if (m_from.x > m_to.x) {
                m_farX += round(ratioX * stepFar);
                x -= round(ratioX * stepX);
                if (x <= m_to.x) {
                    x = m_to.x;
                    m_farX = savedFar;
                }
            }
        }
        elapsed = static_cast<double>(nowMs - m_stepMs);
        m_stepMs = nowMs;
        stepX = round(elapsed * (speed == 0 ? pathSpeed : speed));
        int y = m_y;
        if (m_distX >= m_distY) {
            const double ratioY = m_distY / (m_distX + kNoZero);
            if (m_from.y < m_to.y) {
                y += round(ratioY * stepX);
                if (y >= m_to.y) {
                    y = m_to.y;
                }
            } else if (m_from.y > m_to.y) {
                y -= round(ratioY * stepX);
                if (y <= m_to.y) {
                    y = m_to.y;
                }
            }
        } else {
            if (m_from.y < m_to.y) {
                y += stepX;
                if (y >= m_to.y) {
                    y = m_to.y;
                }
            } else if (m_from.y > m_to.y) {
                y -= stepX;
                if (y <= m_to.y) {
                    y = m_to.y;
                }
            }
        }
        moveTo(x, y);
    }

    void BattleCamera::loadPath(const std::vector<formats::MotionCameraStep>& steps, int side) {
        constexpr int kEdge = 32; // px added to a path point's x
        constexpr int kShift = 50; // px the path x is shifted back
        constexpr int kDown = 40; // px added to a path point's y
        m_path.clear();
        for (const formats::MotionCameraStep& step : steps) {
            int x = side == 1 ? m_x - (step.point.x + kEdge) + m_x + kShift : step.point.x + kEdge - kShift;
            if (x <= 0) {
                x = 0;
            } else if (x + kViewWidth > kGroundWidth) {
                x = kGroundWidth - kViewWidth;
            }
            m_path.push_back(PathStep{{x, step.point.y + kDown}, step.speed, step.until, step.focus});
        }
    }

    void BattleCamera::endPathAtRest() {
        if (!m_path.empty()) {
            m_path.back().point = kRest;
        }
    }

    int BattleCamera::focus(int step) const { return step >= 1 && step <= pathSteps() ? m_path[static_cast<std::size_t>(step - 1)].focus : 0; }

    bool BattleCamera::stepDone(int until, int motionPoint, std::uint64_t nowMs) const {
        if (until > 0 && until < 101) { // values below 101 are motion steps, larger ones milliseconds
            return motionPoint >= until;
        }
        return static_cast<double>(nowMs - m_pathMs) >= until;
    }

    void BattleCamera::followPath(int motionPoint, std::uint64_t nowMs) {
        if (pathSteps() < m_pathStep) {
            return;
        }
        const PathStep& current = m_path[static_cast<std::size_t>(m_pathStep - 1)];
        if (m_stepMoving) {
            startMove({m_x, m_y}, current.point);
            step(0, nowMs);
            if (atTarget()) {
                m_stepArrived = true;
                m_stepMoving = false;
                m_moving = false;
                m_pathMs = nowMs;
            }
        }
        if (m_stepArrived && stepDone(current.until, motionPoint, nowMs)) {
            ++m_pathStep;
            m_stepArrived = false;
            m_stepMoving = true;
            m_moveMs = nowMs;
            m_stepMs = nowMs;
        }
    }

    void BattleCamera::jumpStep(core::Point centre, int motionPoint, std::uint64_t nowMs) {
        if (m_pathStep > pathSteps()) {
            return;
        }
        if (m_jumpReady) {
            moveTo(centre.x - kHalfWidth, centre.y - kHalfHeight);
            m_jumpReady = false;
            m_jumpWaiting = true;
        }
        if (m_jumpWaiting && stepDone(m_path[static_cast<std::size_t>(m_pathStep - 1)].until, motionPoint, nowMs)) {
            m_jumpReady = true;
            m_jumpWaiting = false;
            ++m_pathStep;
            m_pathMs = nowMs;
        }
    }

    int BattleCamera::followStepX(
        int current,
        int target,
        std::uint64_t startMs,
        double pausedMs,
        int speedMode,
        int moveKind,
        std::uint64_t nowMs
    ) {
        double elapsed = static_cast<double>(nowMs) - static_cast<double>(startMs);
        if (pausedMs > 0 && speedMode != 2) {
            elapsed -= pausedMs;
        }
        int stepX = 0;
        int stepFar = 0;
        if (speedMode == 0 || speedMode == 2) {
            // camera speed in px per ms (0.8 for move kind 1, else 0.6); the far layer takes half
            stepX = round((moveKind == 1 ? 0.8 : 0.6) * elapsed);
            stepFar = round((moveKind == 1 ? 0.4 : 0.3) * elapsed);
        } else if (speedMode == 1) {
            stepX = round(elapsed * m_slope);
            stepFar = round(m_slope / 2 * elapsed);
        }
        int position = current;
        const int savedFar = m_farX;
        if (m_distX >= m_distY) {
            if (current < target) {
                m_farX += stepFar;
                position += stepX;
                if (position > target) {
                    position = target;
                    m_farX = savedFar;
                }
            } else if (current > target) {
                m_farX -= stepFar;
                position -= stepX;
                if (position < target) {
                    position = target;
                    m_farX = savedFar;
                }
            }
        } else {
            const double ratio = m_distX / (m_distY + kNoZero);
            if (current < target) {
                m_farX += round(ratio * stepFar);
                position += round(ratio * stepX);
            } else if (current > target) {
                m_farX -= round(ratio * stepFar);
                position -= round(ratio * stepX);
            }
        }
        return position;
    }

    int BattleCamera::followStepY(
        int current,
        int target,
        std::uint64_t startMs,
        double pausedMs,
        int speedMode,
        int moveKind,
        std::uint64_t nowMs
    ) const {
        double elapsed = static_cast<double>(nowMs) - static_cast<double>(startMs);
        if (pausedMs > 0 && speedMode != 2) {
            elapsed -= pausedMs;
        }
        int step = 0;
        if (speedMode == 0 || speedMode == 2) {
            step = round((moveKind == 1 ? 0.8 : 0.6) * elapsed); // camera speed in px per ms (0.8 for move kind 1, else 0.6)
        } else if (speedMode == 1) {
            step = round(elapsed * m_slope);
        }
        int position = current;
        if (m_distX > m_distY) {
            const double ratio = m_distY / (m_distX + kNoZero);
            if (current < target) {
                position += round(ratio * step);
            } else if (current > target) {
                position -= round(ratio * step);
            }
        } else {
            if (current < target) {
                position += step;
                if (position > target) {
                    position = target;
                }
            } else if (current > target) {
                position -= step;
                if (position < target) {
                    position = target;
                }
            }
        }
        return position;
    }

}
