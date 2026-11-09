#include "ui/Window.h"

namespace ui {

    namespace {

        constexpr int kSnap = 10; // drag snaps to the left/top view edge within this distance, px
        constexpr int kSnapRight = 10; // snap distance to the right view edge, px
        constexpr int kSnapBottom = 50; // snap distance to the bottom view edge, px (taskbar area)
        constexpr int kBottomEdge = 40; // gap left above the bottom view edge after snapping, px
        constexpr int kTitleHeight = 24; // height of the title strip that shows the window hint, px
        constexpr int kFormBottom = 60; // screen-fit window drag: bottom zone that triggers the clamp, px
        constexpr int kFormBottomEdge = 50; // screen-fit window drag: gap left above the bottom edge, px
        constexpr int kFitBottom = 35; // screen-fit: bottom zone that triggers the clamp, px
        constexpr int kFitBottomEdge = 25; // screen-fit: gap left above the bottom edge, px

    }

    Window::Window() {
        declareEvent("Click");
        declareEvent("DoubleClick");
        declareEvent("Escape");
        declareEvent("MouseDown");
        declareEvent("MouseUp");
    }

    void Window::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Picture") {
            m_pictureName = value;
        } else if (name == "ScreenFit") {
            m_screenFit = LoadContext::parseBool(value);
        } else if (name == "AbovePopups") {
            m_abovePopups = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void Window::finishLoading(const LoadContext& context) {
        m_picture = context.picture(m_pictureName);
        if (m_picture.isValid() && !sizeWasGiven()) {
            const gfx::Picture& picture = context.pictureData(m_picture);
            setBounds({bounds().x, bounds().y, picture.width(), picture.height()});
        }
        Element::finishLoading(context);
    }

    void Window::setPicture(const std::string& name, gfx::PictureLibrary& pictures) {
        m_pictureName = name;
        m_picture = pictures.find(name).value_or(gfx::PictureId{});
        if (m_picture.isValid()) {
            const gfx::Picture& picture = pictures.picture(m_picture);
            setBounds({bounds().x, bounds().y, picture.width(), picture.height()});
        }
    }

    bool Window::hitTest(core::Point local, const gfx::PictureLibrary&) const {
        return local.x >= 0 && local.y >= 0 && local.x < bounds().width && local.y < bounds().height;
    }

    const std::string& Window::hintAt(core::Point local) const {
        static const std::string none;
        return local.y < kTitleHeight ? hint() : none;
    }

    void Window::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left) {
            m_view = context.view;
            m_grab = local;
            m_pressed = true;
            snapToEdges();
            raise(context, "MouseDown", -1, local);
        }
        Element::onMouseDown(context, button, local);
    }

    void Window::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        Element::onMouseUp(context, button, local);
        if (button == MouseButton::Left) {
            m_pressed = false;
            raise(context, "MouseUp", -1, local);
            raise(context, "Click");
        }
    }

    void Window::onCaptureReleased(UiContext&) { m_pressed = false; }

    void Window::onDoubleClick(UiContext& context, core::Point) { raise(context, "DoubleClick"); }

    void Window::onMouseDrag(UiContext&, core::Point local) {
        core::Rect area = bounds();
        area.x += local.x - m_grab.x;
        area.y += local.y - m_grab.y;
        if (m_screenFit) {
            if (area.y < m_view.top() + kSnap) {
                area.y = m_view.top();
            }
            if (area.y + area.height > m_view.bottom() - kFormBottom) {
                area.y = m_view.bottom() - kFormBottomEdge - area.height;
            }
            setBounds(area);
            return;
        }
        setBounds(area);
        snapToEdges();
    }

    void Window::snapToEdges() {
        if (m_screenFit) {
            return;
        }
        core::Rect area = bounds();
        if (area.x < m_view.left() + kSnap) {
            area.x = m_view.left();
        }
        if (area.x + area.width > m_view.right() - kSnapRight) {
            area.x = m_view.right() - area.width;
        }
        if (area.y < m_view.top() + kSnap) {
            area.y = m_view.top();
        }
        if (area.y + area.height > m_view.bottom() - kSnapBottom) {
            area.y = m_view.bottom() - kBottomEdge - area.height;
        }
        setBounds(area);
    }

    void Window::fitToScreen() {
        if (!m_screenFit) {
            return;
        }
        core::Rect area = bounds();
        if (area.x < m_view.left() + kSnap) {
            area.x = m_view.left();
        }
        if (area.x + area.width > m_view.right() - kSnapRight && area.x > m_view.left()) {
            area.x = m_view.right() - area.width;
        }
        if (area.y + area.height > m_view.bottom() - kFitBottom) {
            area.y = m_view.bottom() - kFitBottomEdge - area.height;
        }
        if (area.y < m_view.top() + kSnap && m_view.bottom() - kFitBottomEdge - area.height != area.y) {
            area.y = m_view.top();
        }
        setBounds(area);
    }

    void Window::update(UiContext& context) {
        m_view = context.view;
        fitToScreen();
        if (m_pressed) {
            snapToEdges();
        }
        Element::update(context);
    }

    void Window::draw(DrawContext& context, core::Point screenPosition) {
        if (m_picture.isValid()) {
            context.canvas.drawPicture(m_picture, screenPosition);
        }
    }

}
