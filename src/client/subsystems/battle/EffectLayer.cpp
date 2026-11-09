#include "client/subsystems/battle/EffectLayer.h"

#include <cmath>
#include <cstdlib>
#include <optional>
#include <utility>

namespace client {

    namespace {

        constexpr double kNoZero = 0.0001; // added to a divisor so it is never zero
        constexpr int kLightStrength = 30; // strength of the additive light blend for translucent effects
        constexpr std::uint16_t kLightMark = 1; // RGB565 value of the picture's top left pixel that marks it as translucent

        /// @brief Rounds a double to the nearest int.
        /// @param value The value.
        /// @return The rounded value.
        int round(double value) { return static_cast<int>(std::lrint(value)); }

        /// @brief Builds an effect picture's name from its id and direction suffix.
        /// @details Original: GetSkillPicName.
        /// @param effectId The effect id.
        /// @param direction Suffix index 0..4 ("", L, R, F, B).
        /// @return The picture name, e.g. "S<id>L".
        std::string pictureName(int effectId, int direction) {
            static const char* const kSuffixes[] = {"", "L", "R", "F", "B"};
            const char* suffix = direction >= 0 && direction <= 4 ? kSuffixes[direction] : ""; // 4: highest direction suffix index
            return "S" + std::to_string(effectId) + suffix;
        }

    }

    void EffectLayer::reset(std::uint64_t nowMs) {
        m_moving = false;
        m_pointIndex = 1;
        m_repeatCounter = 1;
        m_frameMs = nowMs;
        m_moveMsX = nowMs;
        m_moveMsY = nowMs;
        m_delayMs = nowMs;
    }

    void EffectLayer::load(const formats::MotionLayer& layer, std::vector<core::Point> points, int direction, core::Point camera) {
        m_name = pictureName(layer.effectId, direction);
        m_points = std::move(points);
        m_frameCount = layer.frameCount;
        m_startFrame = layer.startFrame;
        m_endFrame = layer.endFrame;
        if (m_picture && m_frameCount != 0) {
            m_frameHeight = round(static_cast<double>(m_picHeight) / m_frameCount);
            m_frameWidth = m_picWidth;
        }
        m_cameraX = camera.x;
        m_cameraY = camera.y;
        const core::Point first = m_points.size() > 1 ? m_points[1] : core::Point{};
        m_posX = first.x;
        m_posY = first.y;
        placeScreen(first.x, first.y);
        updateBounds();
        m_stepPerMs = layer.slope;
        m_frameDelay = layer.frameDelayMs;
        m_playMode = layer.playMode;
        m_moveMode = layer.moveMode;
        m_reverse = m_startFrame > m_endFrame;
        m_curFrame = m_startFrame;
        showFrame();
        m_startDelay = layer.startDelay;
        m_state = State::Waiting;
        m_repeatCount = layer.repeatCount;
        m_arcHeight = layer.arcHeight;
        m_arcDirection = layer.arcDirection;
        m_effectId = layer.effectId;
        m_picDirection = direction;
        if (isCutIn()) {
            m_startFrame = 1;
            m_endFrame = m_frameCount;
            m_playMode = 0;
            m_moveMode = 0;
            m_repeatCount = 1;
            m_arcHeight = 0;
            m_arcDirection = 0;
            m_picDirection = 0;
        }
    }

    void EffectLayer::loadPicture(gfx::PictureLibrary& pictures) {
        m_picture = pictures.find(m_name);
        if (!m_picture) {
            return;
        }
        const gfx::Picture& picture = pictures.picture(*m_picture);
        m_picHeight = picture.height();
        m_picWidth = picture.width();
        m_translucent = picture.topLeft().toRgb565() == kLightMark;
    }

    void EffectLayer::start(gfx::PictureLibrary& pictures, std::uint64_t nowMs) {
        loadPicture(pictures);
        if (m_frameCount != 0 && m_picture) {
            m_frameHeight = round(static_cast<double>(m_picHeight) / m_frameCount);
            m_frameWidth = m_picWidth;
        }
        m_state = State::Playing;
        m_placed = false;
        reset(nowMs);
    }

    void EffectLayer::update(gfx::PictureLibrary& pictures, int motionPoint, core::Point camera, std::uint64_t nowMs) {
        if (m_state == State::Playing) {
            stepFrames(nowMs);
        }
        m_cameraX = camera.x;
        m_cameraY = camera.y;
        move(pictures, nowMs);
        m_placed = m_state == State::Playing;
        if (m_state != State::Waiting) {
            return;
        }
        if (m_startDelay > 0 && m_startDelay < 101) { // start delays 1..100 are steps of the motion
            if (motionPoint >= m_startDelay) {
                start(pictures, nowMs);
            }
        } else if (static_cast<double>(nowMs - m_delayMs) >= m_startDelay) {
            start(pictures, nowMs);
        }
    }

    bool EffectLayer::framesDue(std::uint64_t nowMs, int& frames) {
        frames = 0;
        const std::uint64_t passed = nowMs - m_frameMs;
        if (m_frameDelay <= 0) {
            frames = 1;
            m_frameMs = nowMs;
            return true;
        }
        if (passed < static_cast<std::uint64_t>(m_frameDelay)) {
            return false;
        }
        frames = static_cast<int>(passed / static_cast<std::uint64_t>(m_frameDelay));
        m_frameMs = nowMs;
        return true;
    }

    void EffectLayer::showFrame() {
        m_source = {0, (m_curFrame - 1) * m_frameHeight, m_frameWidth, m_frameHeight};
        if (m_source.y + m_source.height > m_picHeight) {
            m_source.y = m_picHeight - m_frameHeight;
            m_source.height = m_frameHeight;
        }
    }

    void EffectLayer::stepFrames(std::uint64_t nowMs) {
        int frames = 0;
        const auto advance = [&] {
            if (!m_reverse) {
                if (m_curFrame > m_frameCount) {
                    m_curFrame = m_frameCount;
                }
                showFrame();
                m_curFrame = m_curFrame + frames > m_frameCount ? m_frameCount + 1 : m_curFrame + frames;
            } else {
                if (m_curFrame < 1) {
                    m_curFrame = 1;
                }
                showFrame();
                m_curFrame = m_curFrame - frames < 1 ? 0 : m_curFrame - frames;
            }
        };
        const bool past = m_reverse ? m_curFrame < m_endFrame : m_curFrame > m_endFrame;
        switch (m_playMode) {
            // play mode 1: round and round, a number of times
            case 1:
                if (past) {
                    if (m_repeatCount != 0 && ++m_repeatCounter > m_repeatCount) {
                        m_curFrame = m_startFrame;
                        m_state = State::Done;
                        return;
                    }
                    m_curFrame = m_startFrame;
                }
                if (framesDue(nowMs, frames)) {
                    advance();
                }
                break;
            // play mode 2: up to the last frame and held there
            case 2:
                if (past) {
                    m_curFrame = m_endFrame;
                }
                if (framesDue(nowMs, frames)) {
                    advance();
                }
                break;
            // play mode 3: once
            case 3:
                if (past) {
                    m_state = State::Done;
                    m_curFrame = m_startFrame;
                    return;
                }
                if (framesDue(nowMs, frames)) {
                    advance();
                }
                break;
            // play mode 4: the same frame
            case 4:
                if (nowMs - m_frameMs > static_cast<std::uint64_t>(std::max(0, m_frameDelay))) {
                    m_frameMs = nowMs;
                    if (m_curFrame > m_frameCount) {
                        m_curFrame = m_frameCount;
                    }
                    showFrame();
                }
                break;
            default: // other play modes: the frames stand still
                break;
        }
    }

    void EffectLayer::placeScreen(int x, int y) {
        m_screenX = x - m_cameraX;
        m_screenY = y - m_cameraY;
    }

    void EffectLayer::updateBounds() { m_bounds = {m_screenX - m_frameWidth / 2, m_screenY - m_frameHeight / 2, m_frameWidth, m_frameHeight}; }

    void EffectLayer::startMove(core::Point from, core::Point to) {
        if (m_moving) {
            return;
        }
        m_distX = std::abs(from.x - to.x);
        m_distY = std::abs(from.y - to.y);
        m_moveTo = to;
        m_moveFrom = from;
        m_moveDistance = round(std::sqrt(static_cast<double>(m_distX * m_distX + m_distY * m_distY)));
        m_moving = true;
    }

    bool EffectLayer::atTarget() const {
        if (m_distX >= m_distY) {
            return m_posX == m_moveTo.x;
        }
        return m_posY == m_moveTo.y;
    }

    void EffectLayer::stepAlone(std::uint64_t nowMs) {
        int step = round(static_cast<double>(nowMs - m_moveMsX) * m_stepPerMs);
        m_moveMsX = nowMs;
        int x = m_posX;
        if (m_distX >= m_distY) {
            if (m_moveFrom.x < m_moveTo.x) {
                x = std::min(x + step, m_moveTo.x);
            } else if (m_moveFrom.x > m_moveTo.x) {
                x = std::max(x - step, m_moveTo.x);
            }
        } else {
            const int part = round(m_distX / (m_distY + kNoZero) * step);
            if (m_moveFrom.x < m_moveTo.x) {
                x = std::min(x + part, m_moveTo.x);
            } else if (m_moveFrom.x > m_moveTo.x) {
                x = std::max(x - part, m_moveTo.x);
            }
        }
        step = round(m_stepPerMs * static_cast<double>(nowMs - m_moveMsY));
        m_moveMsY = nowMs;
        int y = m_posY;
        if (m_distX >= m_distY) {
            const int part = round(m_distY / (m_distX + kNoZero) * step);
            if (m_moveFrom.y < m_moveTo.y) {
                y = std::min(y + part, m_moveTo.y);
            } else if (m_moveFrom.y > m_moveTo.y) {
                y = std::max(y - part, m_moveTo.y);
            }
        } else {
            if (m_moveFrom.y < m_moveTo.y) {
                y = std::min(y + step, m_moveTo.y);
            } else if (m_moveFrom.y > m_moveTo.y) {
                y = std::max(y - step, m_moveTo.y);
            }
        }
        placeScreen(x, y);
        updateBounds();
        m_posX = x;
        m_posY = y;
    }

    void EffectLayer::arcStep(std::uint64_t nowMs) {
        stepAlone(nowMs);
        const int remain = round(std::sqrt(static_cast<double>((m_moveTo.x - m_posX) * (m_moveTo.x - m_posX) + (m_moveTo.y - m_posY) * (m_moveTo.y - m_posY))));
        // 0.001 keeps the divisor above zero; 360 is the peak height scale of the arc; 0.003 of the squared rise is taken off
        m_arcRise = round((0.5 - std::abs(remain / (m_moveDistance + 0.001) - 0.5)) * 360);
        m_arcRise = round(m_arcRise - static_cast<double>(m_arcRise) * m_arcRise * 0.003);
        m_arcRise = round(m_arcRise * static_cast<double>(m_arcHeight));
        m_screenY += m_arcDirection == 1 ? m_arcRise : (m_arcDirection == 0 ? -m_arcRise : 0);
        updateBounds();
    }

    /// @brief The picture codes and suffixes an arrow takes for one way of travel and bend.
    /// @details Original: UpdateEffectPicDirection. The codes are for the bands of the angle it moves at, climbing or falling.
    struct ArrowCase {
        int codes[4][2];
        int suffix;
        int picDir;
        int lastSuffix;
        int lastPicDir;
        bool before;
        int low;
        int high;
        int falling;
        int otherwise;
    };

    // per way (right, left) and bend (up, down): picture codes by angle band (climbing, falling), suffix, FPicDir, flat band data
    // codes[band] = {code when moving down (deltaY >= 0), code when moving up}; the band is the angle band 0 (steepest) to 3;
    // the numbers are the last two digits of the arrow's effect id (picture S<base><code>); low / high / falling / otherwise
    // are such codes for the flat band: before the mark when the arc is low (height 0..3) / high (4..10) / the effect falls, after it
    const ArrowCase kArrowCases[2][2] = {
        {{{{8, 16}, {7, 1}, {6, 2}, {5, 3}}, 1, 1, 1, 2, true, 4, 1, 16, 4}, {{{8, 16}, {9, 15}, {10, 14}, {11, 13}}, 2, 2, 2, 2, true, 12, 9, 9, 12}},
        {{{{8, 16}, {7, 1}, {6, 2}, {5, 3}}, 2, 2, 2, 2, false, 4, 1, 1, 4}, {{{8, 16}, {9, 15}, {10, 14}, {11, 13}}, 1, 1, 1, 1, false, 12, 9, 9, 12}}
    };

    void EffectLayer::turnStep(gfx::PictureLibrary& pictures, std::uint64_t nowMs) {
        const int xOrder = m_points[1].x <= m_points[2].x ? 1 : 2;
        const int yOrder = m_points[1].y <= m_points[2].y ? 1 : 2;
        const int beforeX = m_screenX;
        const int beforeY = m_screenY;
        arcStep(nowMs);
        const int deltaX = m_screenX - beforeX;
        const int deltaY = m_screenY - beforeY;
        const double cosine = std::cos(deltaY / (deltaX + kNoZero));
        const std::string text = std::to_string(m_effectId);
        // the effect number's digits 1..3 are the base, digits 4..5 the code
        const int ownCode = text.size() >= 5 ? std::atoi(text.substr(3, 2).c_str()) : 0;
        const int base = text.size() >= 3 ? std::atoi(text.substr(0, 3).c_str()) : 0;
        const ArrowCase& arrow = kArrowCases[xOrder - 1][m_arcDirection == 1 ? 1 : 0];
        std::optional<int> code;
        int suffix = arrow.suffix;
        int picDir = arrow.picDir;
        // 0.866 (cos 30 degrees) starts the nearly flat band; 0.1, 0.25 and 0.707 split the steeper bands
        if (cosine >= 0 && cosine < 0.866) {
            const int band = cosine < 0.1 ? 0 : (cosine < 0.25 ? 1 : (cosine < 0.707 ? 2 : 3));
            code = arrow.codes[band][deltaY >= 0 ? 0 : 1];
        } else if (cosine >= 0.866 && cosine <= 1.0) {
            suffix = arrow.lastSuffix;
            picDir = arrow.lastPicDir;
            const int span = round(static_cast<double>(m_distX / 10) * (m_arcHeight * 2 / 10)); // 10: tenths of the way and of the arc height
            const int height = round(static_cast<double>(m_arcHeight) * 2);
            const int mark = xOrder == 1 ? m_points[1].x + span - m_cameraX : m_points[1].x - span - m_cameraX;
            if (arrow.before ? m_screenX < mark : m_screenX > mark) {
                if (yOrder == 2) {
                    code = arrow.falling;
                } else if (height >= 0 && height <= 3) { // 0..3: a low arc
                    code = arrow.low;
                } else if (height >= 4 && height <= 10) { // 4..10: a high arc
                    code = arrow.high;
                }
            } else {
                code = arrow.otherwise;
            }
        }
        if (!code || (ownCode == *code && m_picDirection == picDir)) {
            return;
        }
        m_name = pictureName(base * 100 + *code, suffix); // 100: the code takes the last two digits of the effect id
        m_picDirection = picDir;
        loadPicture(pictures);
        if (m_frameCount != 0 && m_picture) {
            m_frameHeight = round(static_cast<double>(m_picHeight) / m_frameCount);
            m_frameWidth = m_picWidth;
        }
        m_curFrame = m_startFrame;
        m_repeatCounter = 1;
        showFrame();
    }

    void EffectLayer::move(gfx::PictureLibrary& pictures, std::uint64_t nowMs) {
        if (m_state != State::Playing) {
            return;
        }
        const int count = static_cast<int>(m_points.size()) - 1;
        const auto point = [&](int index) { return index >= 1 && index <= count ? m_points[static_cast<std::size_t>(index)] : core::Point{}; };
        if (m_moveMode == 0) {
            placeScreen(point(m_pointIndex).x, point(m_pointIndex).y);
            updateBounds();
        } else if (m_moveMode == 1) {
            if (count > 1 && m_pointIndex < count) {
                startMove(point(m_pointIndex), point(m_pointIndex + 1));
                stepAlone(nowMs);
                if (atTarget()) {
                    ++m_pointIndex;
                    m_moving = false;
                    if (m_pointIndex >= count) {
                        m_moveMode = 0;
                        m_pointIndex = count;
                    }
                }
            }
        } else if (m_moveMode == 2) { // move mode 2: travel once from point to point, then done
            if (count > 1 && m_pointIndex < count) {
                startMove(point(m_pointIndex), point(m_pointIndex + 1));
                const char kind = m_name.size() > 1 ? m_name[1] : ' ';
                if (kind == '1') {
                    arcStep(nowMs);
                } else if (kind == '5') {
                    turnStep(pictures, nowMs);
                }
                if (atTarget()) {
                    ++m_pointIndex;
                    m_moving = false;
                    if (m_pointIndex > count) {
                        m_pointIndex = count;
                    }
                    m_state = State::Done;
                }
            }
        }
    }

    void EffectLayer::draw(gfx::Canvas& canvas, core::Point shake) const {
        if (!m_picture || m_state != State::Playing) {
            return;
        }
        if (m_bounds.right() <= 0 && m_bounds.bottom() <= 0) {
            return;
        }
        if (m_source.width <= 0 || m_source.height <= 0) {
            return;
        }
        core::Rect source = m_source;
        if (source.y + source.height > m_picHeight) {
            source.y = m_picHeight - m_frameHeight;
            source.height = m_frameHeight;
        }
        if (source.x + source.width > m_picWidth) {
            source.x = 0;
            source.width = m_picWidth;
        }
        const core::Point at{m_bounds.x + shake.x, m_bounds.y + shake.y};
        if (m_translucent) {
            canvas.drawPictureLight(*m_picture, at, kLightStrength, source);
        } else {
            canvas.drawPicture(*m_picture, at, source);
        }
    }

}
