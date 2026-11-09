#include "client/rendering/world/EmotionBubble.h"

#include "core/Size.h"
#include "gfx/PictureLibrary.h"

#include <string>

namespace client {

    namespace {

        constexpr std::uint64_t kFrameMs = 200; // time per animation frame, ms
        constexpr int kLoops = 4; // times the animation plays before it disappears
        constexpr int kAboveHead = 20; // px above the top of the body
        constexpr int kLeft = 12; // px left of the feet x

        /// @brief Where an emotion sits beside the head.
        /// @param emotionId Emotion id.
        /// @return Pixel offset from the base position.
        core::Point offsetOf(int emotionId) {
            if (emotionId >= 101 && emotionId <= 111) { // emotion ids 101 to 111
                return {30, -5}; // pixel offset beside the head
            }
            if (emotionId == 112 || emotionId == 113 || emotionId == 116) { // emotion ids 112, 113 and 116
                return {7, 22}; // pixel offset beside the head
            }
            if (emotionId == 114 || emotionId == 115) { // emotion ids 114 and 115
                return {20, 0}; // pixel offset beside the head
            }
            if (emotionId >= 117 && emotionId <= 119) { // emotion ids 117 to 119
                return {-2, -25}; // pixel offset beside the head
            }
            if (emotionId == 201 || emotionId == 202) { // emotion ids 201 and 202 (the large hearts)
                return {30, -5}; // pixel offset beside the head
            }
            return {0, 0};
        }

        /// @brief The frame size of an emotion's strip: the large hearts' are 39 x 36, the others' 32 x 32.
        /// @param emotionId Emotion id.
        /// @return Frame size in pixels.
        core::Size frameOf(int emotionId) {
            return emotionId == 201 || emotionId == 202 ? core::Size{39, 36} : core::Size{32, 32};
        } // frame sizes in pixels: large hearts (ids 201, 202) and the others

    }

    void EmotionBubble::show(int emotionId, std::uint64_t nowMs) {
        m_emotionId = emotionId;
        m_frame = 0;
        m_loops = 1;
        m_frameMs = nowMs;
    }

    void EmotionBubble::draw(gfx::Canvas& canvas, core::Point feet, int height, std::uint64_t nowMs) {
        if (m_emotionId == 0) {
            return;
        }
        const auto picture = canvas.pictures().find("E" + std::to_string(m_emotionId));
        if (!picture) {
            m_emotionId = 0;
            return;
        }
        const core::Point offset = offsetOf(m_emotionId);
        const core::Size frame = frameOf(m_emotionId);
        const core::Point at{feet.x - kLeft + offset.x, feet.y - height - kAboveHead + offset.y};
        canvas.drawPicture(*picture, at, core::Rect{0, m_frame * frame.height, frame.width, frame.height});
        if (nowMs < m_frameMs + kFrameMs) {
            return;
        }
        ++m_frame;
        if (canvas.pictures().picture(*picture).height() / frame.height - 1 < m_frame) {
            if (++m_loops > kLoops) {
                m_emotionId = 0;
                return;
            }
            m_frame = 0;
        }
        m_frameMs = nowMs;
    }

}
