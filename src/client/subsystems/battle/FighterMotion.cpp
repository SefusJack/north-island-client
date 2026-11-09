#include "client/subsystems/battle/FighterMotion.h"

#include "client/subsystems/battle/Fighter.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace client {

    namespace {

        constexpr double kNoZero = 0.0001;

        /// @brief Rounds a double to the nearest int.
        /// @param value The value.
        /// @return The rounded value.
        int round(double value) { return static_cast<int>(std::lrint(value)); }

        /// @brief Reinterprets 32 bits as a single-precision float (a Delphi Single stored in the data).
        /// @param bits The raw bits.
        /// @return The float value.
        float singleOf(std::int32_t bits) {
            float value = 0;
            std::memcpy(&value, &bits, sizeof value);
            return value;
        }

        /// @brief Mirrors a caster motion's pose slot for side 1.
        /// @details Original: SetupCasterMotion's mirroring of the eight-way slots.
        /// @param slot The pose slot.
        /// @return The mirrored slot.
        int casterMirror(int slot) {
            if (slot >= 0 && slot <= 15) { // 0..15: the two sets of eight-way slots
                switch (slot) {
                    case 1: // slot 1: mirrors to slot + 6
                    case 9: // slot 9: mirrors to slot + 6
                        return slot + 6;
                    case 2: // slot 2: mirrors to slot + 4
                    case 10: // slot 10: mirrors to slot + 4
                        return slot + 4;
                    case 3: // slot 3: mirrors to slot + 2
                    case 11: // slot 11: mirrors to slot + 2
                        return slot + 2;
                    case 5: // slot 5: mirrors to slot - 2
                    case 13: // slot 13: mirrors to slot - 2
                        return slot - 2;
                    case 6: // slot 6: mirrors to slot - 4
                    case 14: // slot 14: mirrors to slot - 4
                        return slot - 4;
                    case 7: // slot 7: mirrors to slot - 6
                    case 15: // slot 15: mirrors to slot - 6
                        return slot - 6;
                    default: // other slots (0, 4, 8, 12) are kept
                        return slot;
                }
            }
            return slot % 2 == 0 ? slot + 1 : slot - 1;
        }

        /// @brief Mirrors a hit motion's pose slot for side 1.
        /// @details Original: SetupBattleMotion's mirroring (the one hit).
        /// @param slot The pose slot.
        /// @return The mirrored slot.
        int hitMirror(int slot) {
            if (slot >= 0 && slot <= 15) { // 0..15: the two sets of eight-way slots
                switch (slot) {
                    case 2: // slot 2: mirrors to slot + 4
                    case 10: // slot 10: mirrors to slot + 4
                        return slot + 4;
                    case 6: // slot 6: mirrors to slot - 4
                    case 14: // slot 14: mirrors to slot - 4
                        return slot - 4;
                    default: // other slots are kept
                        return slot;
                }
            }
            return slot % 2 == 0 ? slot + 1 : slot - 1;
        }

    }

    void FighterMotion::reset(std::uint64_t nowMs) {
        m_arrived = false;
        m_pathAdvanced = false;
        m_segStarted = false;
        m_pathDone = false;
        m_pathEndReached = false;
        m_cameraFollow = false;
        m_running = false;
        m_speedMode = 0;
        m_pathIndex = 0;
        m_pointIndex = 0;
        m_groundPointIndex = 0;
        m_groundSegIndex = 1;
        m_moveStartMs = nowMs;
        m_moveStartMsY = nowMs;
        m_pathStepMs = nowMs;
        m_afterImages = {};
    }

    void FighterMotion::clearAfterImages(std::uint64_t nowMs) {
        m_afterImages = {};
        m_frameTickMs = nowMs;
    }

    void FighterMotion::leaveAfterImage(const Fighter& fighter, std::uint64_t nowMs) {
        const auto slopeAt = [&](int point) {
            return point >= 0 && point < static_cast<int>(m_slopes.size()) ? m_slopes[static_cast<std::size_t>(point)] : 0.0;
        };
        const bool atEnd = m_pathEndReached && m_pathDone;
        const int past = atEnd && m_pointIndex > m_lastPointIndex ? m_pointIndex - 1 : m_pointIndex;
        if (!m_frameTimed) {
            switch (m_speedMode) {
                case 0: // speed mode 0: running or jumping to the first point
                    m_frameDelay =
                        m_moveKind1 == 0 ? 50 : (m_moveKind1 == 1 ? 38 : m_frameDelay); // ghost delay in ms: 50 for a run (kind 0), 38 for a jump (kind 1)
                    break;
                case 1: // speed mode 1: along the points
                {
                    double slope =
                        slopeAt(m_pointIndex) == 0 ? 1 : (atEnd && m_pointIndex > m_lastPointIndex ? slopeAt(m_pointIndex - 1) : slopeAt(m_pointIndex));
                    m_frameDelay = static_cast<int>(std::nearbyint(30 / (slope == 0 ? 1 : slope))); // 30: ghost delay in ms at slope 1
                } break;
                case 2: // speed mode 2: going back to the cell
                    m_frameDelay =
                        m_moveKind2 == 0 ? 50 : (m_moveKind2 == 1 ? 38 : m_frameDelay); // ghost delay in ms: 50 for a run (kind 0), 38 for a jump (kind 1)
                    break;
                default: // other speed modes: delay unchanged
                    break;
            }
            m_frameDelay = std::max(m_frameDelay, 40); // 40: shortest ghost delay in ms
            m_frameTimed = true;
        }
        if (static_cast<double>(nowMs - m_frameTickMs) < m_frameDelay) {
            return;
        }
        m_frameTickMs = nowMs;
        m_frameTimed = false;
        AfterImage image;
        image.position = {m_posX, m_posY};
        if (m_speedMode == 1) {
            image.slot = m_frameSlots[static_cast<std::size_t>(past)];
            image.frame = m_frameHolds[static_cast<std::size_t>(past)];
        } else {
            image.slot = fighter.action;
            image.frame = fighter.drawnFrame;
        }
        image.alpha = 200; // 200: starting opacity of a ghost (of 255)
        // 30 / 30.0: fade scales with the frame delay against 30 ms; 10: extra fade per step
        image.fade = static_cast<int>(std::nearbyint(30 / (m_frameDelay / 30.0))) + 10;
        image.active = true;
        for (std::size_t i = 1; i < m_afterImages.size(); ++i) {
            if (!m_afterImages[i].active) {
                m_afterImages[i] = image;
                return;
            }
        }
        m_afterImages[1] = image;
    }

    void FighterMotion::placeAt(core::Point screen, core::Point camera) {
        m_cameraX = camera.x;
        m_cameraY = camera.y;
        m_posX = screen.x + m_cameraX;
        m_posY = screen.y + m_cameraY;
        m_screenX = screen.x;
        m_screenY = screen.y;
    }

    void FighterMotion::load(const formats::MotionRecord& record, int side, core::Point move, bool caster) {
        m_jumpScale1 = singleOf(record.jumpScale1);
        m_jumpScale2 = singleOf(record.jumpScale2);
        m_pathPointCount = record.pathPointCount;
        m_lastPointIndex = static_cast<int>(record.frames.size());
        m_moveKind1 = record.moveKind1;
        m_moveKind2 = record.moveKind2;
        m_drawPosMode = record.groundMode;
        const std::size_t points = record.frames.size();
        // + 2: the per-point arrays are 1-based (index 0 unused) with one spare entry
        m_frameSlots.assign(points + 2, 0);
        m_frameHolds.assign(points + 2, 0);
        m_slopes.assign(points + 2, 0);
        m_pointDurations.assign(points + 2, 0);
        for (std::size_t i = 0; i < points; ++i) {
            const formats::MotionFrame& frame = record.frames[i];
            m_frameSlots[i + 1] = side == 1 ? (caster ? casterMirror(frame.slot) : hitMirror(frame.slot)) : frame.slot;
            m_frameHolds[i + 1] = frame.hold;
            m_slopes[i + 1] = frame.slope;
            m_pointDurations[i + 1] = frame.durationMs;
        }
        const auto placed = [&](core::Point point) -> core::Point {
            if (side == 1) {
                return {m_start.x - point.x - move.x, point.y + m_start.y - move.y};
            }
            return {point.x + m_start.x + move.x, point.y + m_start.y - move.y};
        };
        const std::size_t pathCount = std::max<std::size_t>(points, static_cast<std::size_t>(std::max(0, m_pathPointCount)));
        const core::Point keepFirst = m_pathPoints.empty() ? core::Point{} : m_pathPoints[0];
        m_pathPoints.assign(pathCount + 2, core::Point{}); // + 2: index 0 is the start, then 1-based, one spare
        m_pathPoints[0] = keepFirst;
        for (int i = 1; i <= m_pathPointCount && static_cast<std::size_t>(i - 1) < record.pathPoints.size(); ++i) {
            m_pathPoints[static_cast<std::size_t>(i)] = placed(record.pathPoints[static_cast<std::size_t>(i - 1)]);
        }
        m_groundSegments.assign(1, {0, 0});
        for (const auto& segment : record.groundSegments) {
            m_groundSegments.push_back(segment);
        }
        m_groundPoints.assign(record.groundPoints.size() + 2, core::Point{}); // + 2: index 0 is the start, then 1-based, one spare
        m_groundPoints[0] = {m_posX, m_posY};
        for (std::size_t i = 0; i < record.groundPoints.size(); ++i) {
            m_groundPoints[i + 1] = placed(record.groundPoints[i]);
        }
        if (caster) {
            m_posX = m_start.x;
            m_posY = m_start.y;
            m_pathPoints[0] = m_start;
            m_targetX = m_start.x;
            m_targetY = m_start.y;
        }
    }

    void FighterMotion::loadStatus(const formats::MotionRecord& record, int side, core::Point cell) {
        placeAt(cell, BattleCamera::kRest);
        m_start = {m_posX, m_posY};
        load(record, side, {}, false);
        m_posX = m_start.x;
        m_posY = m_start.y;
        m_pathPoints[0] = m_start;
        m_targetX = m_start.x;
        m_targetY = m_start.y;
    }

    int FighterMotion::frameSlot(int point) const {
        return point >= 0 && point < static_cast<int>(m_frameSlots.size()) ? m_frameSlots[static_cast<std::size_t>(point)] : 0;
    }

    int FighterMotion::frameHold(int point) const {
        return point >= 0 && point < static_cast<int>(m_frameHolds.size()) ? m_frameHolds[static_cast<std::size_t>(point)] : 0;
    }

    double FighterMotion::slope(int point) const {
        return point >= 0 && point < static_cast<int>(m_slopes.size()) ? m_slopes[static_cast<std::size_t>(point)] : 0;
    }

    void FighterMotion::checkArrived() {
        if (m_segDX >= m_segDY) {
            if (std::abs(m_posX - m_targetX) <= 0) {
                m_arrived = true;
            }
        } else if (std::abs(m_posY - m_targetY) <= 0) {
            m_arrived = true;
        }
    }

    void FighterMotion::advancePoint(std::uint64_t nowMs) {
        if (m_pointIndex != m_pathIndex) {
            return;
        }
        const int duration =
            m_pointIndex >= 0 && m_pointIndex < static_cast<int>(m_pointDurations.size()) ? m_pointDurations[static_cast<std::size_t>(m_pointIndex)] : 0;
        if (static_cast<double>(nowMs - m_pathStepMs) < duration) {
            return;
        }
        if (m_speedMode != 2) { // speed mode 2: going back, keeps no pause
            m_pausedMs = static_cast<double>(nowMs - m_pathStepMs);
            m_lastElapsed = m_pausedMs;
        }
        m_pathStepMs = nowMs;
        ++m_pointIndex;
        m_timedStepOn = false;
        if (m_pointIndex > m_lastPointIndex) {
            m_pointIndex = m_pathEndReached ? m_lastPointIndex + 1 : m_lastPointIndex;
            m_pathDone = true;
        }
    }

    bool FighterMotion::atCameraEdgeX(const BattleCamera& camera) {
        const core::Point view = camera.position();
        constexpr int kHalf = BattleCamera::kHalfWidth;
        if (view.x <= 0) {
            if (m_screenX < kHalf || m_targetX < m_posX) {
                m_horzAnchor = 1; // horizontal anchor 1: the motion moves on its own near the left edge
                return true;
            }
        } else if (view.x + BattleCamera::kViewWidth >= camera.groundWidth()) {
            if (view.x + kHalf < m_posX || (view.x + kHalf <= m_posX && m_targetX > m_posX)) {
                m_horzAnchor = 2; // horizontal anchor 2: the motion moves on its own near the right edge
                return true;
            }
        } else if (m_posX - view.x - kHalf > 0) {
            if (m_targetX <= m_posX) {
                m_horzAnchor = 2; // horizontal anchor 2: the motion moves on its own on the right of the screen centre
                return true;
            }
        } else if (camera.groundWidth() / 2 > m_posX && m_posX - view.x < kHalf && m_targetX >= m_posX) {
            m_horzAnchor = 1; // horizontal anchor 1: the motion moves on its own on the left of the screen centre
            return true;
        }
        return false;
    }

    bool FighterMotion::atCameraEdgeY(const BattleCamera& camera, int jumpHeight) {
        const core::Point view = camera.position();
        constexpr int kHalf = BattleCamera::kHalfHeight;
        if (view.y <= 0) {
            if (m_screenY - jumpHeight < kHalf || (m_screenY - jumpHeight == kHalf && m_targetY < m_posY)) {
                m_anchorMode = 3; // vertical anchor mode 3: the motion moves on its own near the top edge
                return true;
            }
        } else if (view.y + BattleCamera::kViewHeight >= camera.groundHeight()) {
            if (view.y + kHalf < m_posY || (view.y + kHalf == m_posY && m_targetY > m_posY)) {
                m_anchorMode = 4; // vertical anchor mode 4: the motion moves on its own near the bottom edge
                return true;
            }
        } else if (m_posY - view.y - kHalf > 0) {
            if (m_targetY < m_posY) {
                m_anchorMode = 4; // vertical anchor mode 4: the motion moves on its own below the screen centre
                return true;
            }
        } else if (m_posY - view.y < kHalf && m_targetY > m_posY) {
            m_anchorMode = 3; // vertical anchor mode 3: the motion moves on its own above the screen centre
            return true;
        }
        return false;
    }

    void FighterMotion::syncCamera(BattleCamera& camera) {
        const double slope = m_pointIndex >= 0 && m_pointIndex < static_cast<int>(m_slopes.size()) ? m_slopes[static_cast<std::size_t>(m_pointIndex)] : 0;
        camera.setFollow(slope, m_segDX, m_segDY);
        m_cameraX = camera.position().x;
        m_cameraY = camera.position().y;
    }

    int FighterMotion::speedStep(double elapsed) const {
        switch (m_speedMode) {
            case 0: // speed mode 0: running (0.6 px/ms) or jumping (0.8 px/ms) to the first point
                return round((m_moveKind1 == 1 ? 0.8 : 0.6) * elapsed);
            case 1: { // speed mode 1: along the points at the point's slope
                const double slope =
                    m_pointIndex >= 0 && m_pointIndex < static_cast<int>(m_slopes.size()) ? m_slopes[static_cast<std::size_t>(m_pointIndex)] : 0;
                return round(elapsed * slope);
            }
            case 2: // speed mode 2: going back, running (0.6 px/ms) or jumping (0.8 px/ms)
                return round((m_moveKind2 == 1 ? 0.8 : 0.6) * elapsed);
            default: // other speed modes: no step
                return 0;
        }
    }

    int FighterMotion::stepTowardX(int position, int target, std::uint64_t nowMs) {
        double elapsed = static_cast<double>(nowMs - m_moveStartMs);
        if (m_pausedMs > 0) {
            if (m_speedMode == 2) {
                m_pausedMs = 0;
            }
            elapsed = std::max(0.0, elapsed - m_pausedMs);
            m_pausedMs = 0;
        }
        m_moveStartMs = nowMs;
        const int step = speedStep(elapsed);
        int next = position;
        if (position == target) {
            return next;
        }
        const int delta = m_segDX >= m_segDY ? step : round(m_segDX / (m_segDY + kNoZero) * step);
        if (position < target) {
            next = std::min(next + delta, target);
        } else {
            next = std::max(next - delta, target);
        }
        return next;
    }

    int FighterMotion::stepTowardY(int position, int target, std::uint64_t nowMs) {
        double elapsed = static_cast<double>(nowMs - m_moveStartMsY);
        if (m_lastElapsed > 0) {
            if (m_speedMode == 2) {
                m_lastElapsed = 0;
            }
            elapsed = std::max(0.0, elapsed - m_lastElapsed);
            m_lastElapsed = 0;
        }
        m_moveStartMsY = nowMs;
        const int step = speedStep(elapsed);
        int next = position;
        if (position == target) {
            return next;
        }
        const int delta = m_segDX >= m_segDY ? round(m_segDY / (m_segDX + kNoZero) * step) : step;
        if (position < target) {
            next = std::min(next + delta, target);
        } else {
            next = std::max(next - delta, target);
        }
        return next;
    }

    void FighterMotion::beginSegment(core::Point from, core::Point to) {
        if (m_segStarted) {
            return;
        }
        m_segDX = std::abs(from.x - to.x);
        m_segDY = std::abs(from.y - to.y);
        m_segLength = round(std::sqrt(static_cast<double>(m_segDX * m_segDX + m_segDY * m_segDY)));
        m_targetX = to.x;
        m_targetY = to.y;
        m_segStarted = true;
    }

    void FighterMotion::screenToPosition(int x, int y) {
        if (m_horzAnchor == 2) { // horizontal anchor 2: right side
            m_posX = x;
        } else if (m_horzAnchor == 1) { // horizontal anchor 1: left side
            m_posX = m_posX < BattleCamera::kHalfWidth ? x + m_cameraX : x;
        } else {
            m_posX = x + m_cameraX;
        }
        m_posY = m_anchorMode == 4 ? y : y + m_cameraY; // vertical anchor mode 4: bottom
    }

    void FighterMotion::anchor(int x, int y) {
        m_posX = x;
        m_screenX = m_horzAnchor == 0 ? BattleCamera::kHalfWidth : x - m_cameraX;
        if (m_anchorMode == 0) {
            m_posY = y;
            m_screenY = BattleCamera::kHalfHeight;
        } else if (m_anchorMode == 3) { // vertical anchor mode 3: top
            m_posY = y;
            m_screenY = y;
        } else if (m_anchorMode == 4) { // vertical anchor mode 4: bottom
            m_posY = y;
            m_screenY = m_posY - m_cameraY;
        }
    }

    void FighterMotion::followWithCamera(BattleCamera& camera, int jumpHeight, std::uint64_t nowMs) {
        int x = 0;
        if (atCameraEdgeX(camera)) {
            m_posX = stepTowardX(m_posX, m_targetX, nowMs);
            x = m_posX;
        } else {
            x = BattleCamera::kHalfWidth;
            const int kind = m_speedMode == 0 ? m_moveKind1 : (m_speedMode == 2 ? m_moveKind2 : 0);
            const int stepX = camera.followStepX(m_posX, m_targetX, m_moveStartMs, m_pausedMs, m_speedMode, kind, nowMs);
            camera.moveTo(stepX - BattleCamera::kHalfWidth, camera.position().y);
            m_pausedMs = 0;
            m_horzAnchor = 0;
            m_moveStartMs = nowMs;
        }
        int y = 0;
        if (atCameraEdgeY(camera, jumpHeight)) {
            m_posY = stepTowardY(m_posY, m_targetY, nowMs);
            y = m_posY;
        } else {
            y = BattleCamera::kHalfHeight;
            const int kind = m_speedMode == 0 ? m_moveKind1 : (m_speedMode == 2 ? m_moveKind2 : 0);
            const int stepY = camera.followStepY(m_posY, m_targetY, m_moveStartMsY, m_lastElapsed, m_speedMode, kind, nowMs);
            camera.moveTo(camera.position().x, stepY - BattleCamera::kHalfHeight);
            m_lastElapsed = 0;
            m_anchorMode = 0;
            m_moveStartMsY = nowMs;
        }
        m_cameraX = camera.position().x;
        m_cameraY = camera.position().y;
        screenToPosition(x, y);
        anchor(m_posX, m_posY);
        checkArrived();
    }

    void FighterMotion::stepAlone(const BattleCamera& camera, std::uint64_t nowMs) {
        m_cameraX = camera.position().x;
        m_cameraY = camera.position().y;
        const int x = stepTowardX(m_posX, m_targetX, nowMs);
        const int y = stepTowardY(m_posY, m_targetY, nowMs);
        m_posX = x;
        m_posY = y;
        m_screenX = m_posX - m_cameraX;
        m_screenY = m_posY - m_cameraY;
        checkArrived();
    }

    int FighterMotion::holdSleepMs() const {
        if (m_pathPointCount <= 0 || m_pathIndex >= m_pathPointCount || m_speedMode != 1 || m_pausedMs != 0) {
            return 0;
        }
        const auto at = static_cast<std::size_t>(m_pathIndex);
        if (at + 1 >= m_pathPoints.size()) {
            return 0;
        }
        return m_pathPoints[at] == m_pathPoints[at + 1] ? kHoldSleepMs : 0;
    }

    void FighterMotion::advancePath(BattleCamera& camera, bool fighterFollowed, int jumpHeight, std::uint64_t nowMs) {
        if (m_pathPointCount > 0) {
            if (m_pathIndex < m_pathPointCount) {
                if (m_speedMode == 1 && m_timedStepOn) {
                    if (!m_cameraFollow) {
                        m_cameraX = camera.position().x;
                        m_cameraY = camera.position().y;
                        m_screenX = m_posX - m_cameraX;
                        m_screenY = m_posY - m_cameraY;
                    }
                    return;
                }
                beginSegment({m_posX, m_posY}, m_pathPoints[static_cast<std::size_t>(m_pathIndex + 1)]);
                if (fighterFollowed && m_cameraFollow) {
                    syncCamera(camera);
                    followWithCamera(camera, jumpHeight, nowMs);
                } else {
                    stepAlone(camera, nowMs);
                }
                if (m_arrived) {
                    m_pathStepMs = nowMs;
                    m_timedStepOn = true;
                    m_arrived = false;
                    m_segStarted = false;
                    if (!m_pathAdvanced) {
                        reset(nowMs);
                        m_pathAdvanced = true;
                        m_speedMode = 1;
                        m_pointIndex = 1;
                    }
                    ++m_pathIndex;
                }
            } else {
                m_cameraX = camera.position().x;
                m_cameraY = camera.position().y;
                m_screenX = m_posX - m_cameraX;
                m_screenY = m_posY - m_cameraY;
            }
        }
        if (m_pathIndex >= m_pathPointCount) {
            m_pathEndReached = true;
        }
    }

    void FighterMotion::calcDrawPosition() {
        const auto pathPoint = [&](int index) {
            return index >= 0 && index < static_cast<int>(m_pathPoints.size()) ? m_pathPoints[static_cast<std::size_t>(index)] : core::Point{};
        };
        switch (m_drawPosMode) {
            case 1: // ground mode 1: the feet are drawn at the motion's screen position
                m_homeX = m_screenX;
                m_homeY = m_screenY;
                break;
            case 0: // ground mode 0: the feet stay at least as low as the first path point
                m_homeX = m_screenX;
                m_homeY = pathPoint(1).y - m_cameraY > m_screenY ? pathPoint(1).y - m_cameraY : m_screenY;
                break;
            case 2: { // ground mode 2: the feet follow the ground segments between path points
                const auto segment = m_groundSegIndex < static_cast<int>(m_groundSegments.size()) ? m_groundSegments[static_cast<std::size_t>(m_groundSegIndex)]
                                                                                                  : std::pair<int, int>{0, 0};
                if (segment.first == 0 || segment.first > m_pathIndex) {
                    m_homeX = m_screenX;
                    m_homeY = m_screenY;
                    break;
                }
                if (segment.second < m_pathIndex) {
                    break;
                }
                const core::Point from = pathPoint(segment.first);
                const core::Point to = pathPoint(segment.second);
                const int segDX = std::abs(from.x - to.x);
                const int segDY = std::abs(from.y - to.y);
                const int done = std::abs(m_posX - from.x);
                m_homeX = m_screenX;
                const float ratio = static_cast<float>(done / static_cast<double>(segDX) + 0.0001); // 0.0001: avoids a zero ratio
                if (segDY == 0) {
                    m_homeY = from.y - m_cameraY;
                } else if (from.y > to.y) {
                    m_homeY = from.y - m_cameraY - round(segDY * ratio);
                } else {
                    m_homeY = from.y - m_cameraY + round(segDY * ratio);
                }
                if (segment.second == m_pathIndex) {
                    m_homeY = to.y;
                    ++m_groundSegIndex;
                }
                break;
            }
            case 3: { // ground mode 3: the feet follow the ground points
                // - 2: drops index 0 (the start) and the spare entry
                const int count = static_cast<int>(m_groundPoints.size()) - 2;
                if (count <= 0) {
                    break;
                }
                if (m_groundPointIndex >= count) {
                    m_homeX = m_screenX;
                    m_homeY = m_screenY;
                    break;
                }
                const core::Point from = m_groundPoints[static_cast<std::size_t>(m_groundPointIndex)];
                const core::Point to = m_groundPoints[static_cast<std::size_t>(m_groundPointIndex + 1)];
                const int segDX = std::abs(from.x - to.x);
                const int segDY = std::abs(from.y - to.y);
                const int done = std::abs(m_posX - from.x);
                m_homeX = m_screenX;
                const float ratio = static_cast<float>(done / static_cast<double>(segDX) + 0.0001); // 0.0001: avoids a zero ratio
                if (segDY == 0) {
                    m_homeY = from.y - m_cameraY;
                } else if (from.y > to.y) {
                    m_homeY = from.y - m_cameraY - round(segDY * ratio);
                } else {
                    m_homeY = from.y - m_cameraY + round(segDY * ratio);
                }
                if (to.y <= m_homeY) {
                    m_homeY = to.y;
                    ++m_groundSegIndex;
                }
                break;
            }
            default: // other ground modes: the feet are left as they are
                break;
        }
    }

    void FighterMotion::jumpArc(Fighter& fighter, float scale) const {
        fighter.jumping = true;
        const int remain = round(std::sqrt(static_cast<double>((m_targetX - m_posX) * (m_targetX - m_posX) + (m_targetY - m_posY) * (m_targetY - m_posY))));
        // 0.001 keeps the divisor above zero; 360 is the peak height scale of the arc; 0.003 of the squared height is taken off
        int height = round((0.5 - std::abs(remain / (m_segLength + 0.001) - 0.5)) * 360);
        height = round(height - static_cast<double>(height) * height * 0.003);
        fighter.jumpHeight = round(height * static_cast<double>(scale));
    }

    int FighterMotion::runPose(const Fighter& fighter, bool towardRight) {
        if (fighter.npc) {
            const int kind = fighter.npc->directionKind;
            const bool walkSlots = kind == 1 || (kind == 2 && fighter.npc->uncatchable == 1); // NPC direction kinds 1 and 2 (uncatchable) use the walking slots
            if (walkSlots) {
                return towardRight ? 6 : 2; // walking slots 6 (right) / 2 (left)
            }
        }
        return towardRight ? 65 : 64; // run poses 65 (right) / 64 (left)
    }

    int FighterMotion::jumpPose(const Fighter& fighter) {
        const bool rightSide = fighter.side() == 1;
        if (fighter.npc && fighter.npc->directionKind == 1) { // NPC direction kind 1
            return rightSide ? 16 : 17; // stance 16 (side 1) / 17
        }
        return rightSide ? 36 : 37; // jump poses 36 (side 1) / 37
    }

    void FighterMotion::runToStart(Fighter& fighter, bool updateFighter) const {
        const core::Point first = m_pathPoints.size() > 1 ? m_pathPoints[1] : m_start;
        if (m_start.x == first.x && m_start.y == first.y) {
            return;
        }
        if (m_moveKind1 == 0) { // move kind 0: run
            fighter.position = {m_screenX, m_screenY};
            fighter.mover.setSpeed(0.6); // 0.6: running speed in px/ms
            if (updateFighter) {
                fighter.holdFrame = 255; // 255: no frame held
                fighter.action = runPose(fighter, first.x > m_posX);
            }
        } else if (m_moveKind1 == 1) { // move kind 1: jump
            jumpArc(fighter, m_jumpScale1);
            fighter.position = {m_screenX, m_screenY};
            if (updateFighter) {
                fighter.holdFrame = 0;
                fighter.mover.setSpeed(0.8); // 0.8: jumping speed in px/ms
                fighter.action = jumpPose(fighter);
            }
        }
    }

    void FighterMotion::returnHome(Fighter& fighter, bool updateFighter, BattleCamera& camera, std::uint64_t nowMs) {
        const core::Point last = m_pathPointCount >= 0 && m_pathPointCount < static_cast<int>(m_pathPoints.size())
                                     ? m_pathPoints[static_cast<std::size_t>(m_pathPointCount)]
                                     : m_start;
        if (last.x == m_start.x && last.y == m_start.y) {
            m_arrived = true;
            return;
        }
        if (m_moveKind2 != 0 && m_moveKind2 != 1) {
            return;
        }
        beginSegment({m_posX, m_posY}, m_start);
        if (m_cameraFollow) {
            syncCamera(camera);
            followWithCamera(camera, fighter.jumpHeight, nowMs);
        } else {
            stepAlone(camera, nowMs);
        }
        if (m_moveKind2 == 1) {
            jumpArc(fighter, m_jumpScale2);
        }
        fighter.position = {m_screenX, m_screenY};
        if (!updateFighter) {
            return;
        }
        if (m_moveKind2 == 0) { // move kind 0: run
            fighter.holdFrame = 255; // 255: no frame held
            fighter.mover.setSpeed(0.6); // 0.6: running speed in px/ms
            fighter.action = runPose(fighter, m_start.x > m_posX);
        } else {
            fighter.holdFrame = 0;
            fighter.mover.setSpeed(0.8); // 0.8: jumping speed in px/ms
            fighter.action = jumpPose(fighter);
        }
    }

}
