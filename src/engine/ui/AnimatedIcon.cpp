#include "ui/AnimatedIcon.h"

#include "gfx/PictureLibrary.h"
#include "ui/DrawContext.h"
#include "ui/LoadContext.h"

namespace ui {

    namespace {

        constexpr std::uint64_t kFrameMs = 200; // ms each frame shows while the mouse hovers
        constexpr int kIdleFrame = 2; // frame index shown when not hovered
        constexpr std::uint8_t kHoverAlpha = 80; // alpha (0..255) of the hover colour overlay

    }

    AnimatedIcon::AnimatedIcon() { declareEvent("Click"); }

    void AnimatedIcon::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Picture") {
            m_pictureName = value;
        } else if (name == "HoverColor") {
            m_hoverColor = LoadContext::parseColor(value);
            m_hoverColor.a = kHoverAlpha;
        } else if (name == "Active") {
            m_active = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void AnimatedIcon::finishLoading(const LoadContext& context) {
        m_picture = context.picture(m_pictureName);
        Element::finishLoading(context);
    }

    void AnimatedIcon::setPicture(gfx::PictureId picture) {
        m_picture = picture;
        m_frame = 0;
    }

    bool AnimatedIcon::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return (m_picture.isValid() || m_active) && core::Rect{0, 0, bounds().width, bounds().height}.contains(local);
    }

    void AnimatedIcon::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        (void)context;
        (void)local;
        m_pressed = button == MouseButton::Left;
    }

    void AnimatedIcon::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        (void)local;
        if (button == MouseButton::Left && m_pressed) {
            m_pressed = false;
            raise(context, "Click");
        }
    }

    void AnimatedIcon::draw(DrawContext& context, core::Point screenPosition) {
        if (!m_picture.isValid() && !m_active) {
            return;
        }
        const int width = bounds().width;
        const int height = bounds().height;
        if (context.hoveredId == id()) {
            context.canvas.fillRectAlpha({screenPosition.x, screenPosition.y, width, height}, m_hoverColor);
        }
        if (!m_picture.isValid()) {
            return;
        }
        const gfx::Picture& picture = context.canvas.pictures().picture(m_picture);
        const int frames = picture.width() > 0 ? picture.height() / picture.width() : 1;
        if (context.hoveredId == id()) {
            if (m_frameMs + kFrameMs < context.nowMs) {
                m_frame = m_frame + 1 > frames - 1 ? 0 : m_frame + 1;
                m_frameMs = context.nowMs;
            }
        } else {
            m_frame = kIdleFrame;
        }
        if (frames <= 2) { // a strip of two or fewer frames does not animate and shows frame 0
            m_frame = 0;
        }
        context.canvas.drawPicture(m_picture, screenPosition, core::Rect{0, m_frame * height, width, height});
    }

}
