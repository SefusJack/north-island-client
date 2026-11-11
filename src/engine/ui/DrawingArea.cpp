#include "ui/DrawingArea.h"

#include "ui/DrawContext.h"
#include "ui/UiContext.h"

namespace ui {

    DrawingArea::DrawingArea() {
        declareEvent("MouseDown");
        declareEvent("RightMouseDown");
        declareEvent("RightMouseUp");
        declareEvent("MouseDrag");
        declareEvent("MouseUp");
        declareEvent("DoubleClick");
        declareEvent("Drop");
        declareEvent("MouseWheel");
    }

    void DrawingArea::onMouseWheel(UiContext& context, int steps) { raise(context, "MouseWheel", steps); }

    void DrawingArea::onDropped(
        UiContext& context,
        int sourceIndex,
        std::optional<std::reference_wrapper<const Element>> target,
        int targetIndex,
        core::Point screenPoint,
        core::Point grab
    ) {
        EventArgs args;
        args.index = sourceIndex;
        args.position = screenPoint;
        args.target = target ? target->get().name() : std::string();
        args.targetIndex = targetIndex;
        args.grab = grab;
        raise(context, "Drop", args);
    }

    void DrawingArea::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "HitTest") {
            m_hitTest = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    bool DrawingArea::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return m_hitTest && core::Rect{0, 0, bounds().width, bounds().height}.contains(local) && (!m_hitTester || m_hitTester(local));
    }

    void DrawingArea::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left) {
            m_lastMouse = local;
            raise(context, "MouseDown", -1, local);
        } else if (button == MouseButton::Right) {
            raise(context, "RightMouseDown", -1, local);
        }
    }

    void DrawingArea::onMouseDrag(UiContext& context, core::Point local) {
        m_lastMouse = local;
        raise(context, "MouseDrag", -1, local);
    }

    void DrawingArea::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        if (button == MouseButton::Left) {
            raise(context, "MouseUp", -1, local);
        } else if (button == MouseButton::Right) {
            raise(context, "RightMouseUp", -1, local);
        }
    }

    void DrawingArea::onDoubleClick(UiContext& context, core::Point local) { raise(context, "DoubleClick", -1, local); }

    void DrawingArea::onCaptureReleased(UiContext& context) { raise(context, "MouseUp", -1, m_lastMouse); }

    void DrawingArea::draw(DrawContext& context, core::Point screenPosition) {
        if (m_painter) {
            m_painter(context.canvas, screenPosition, context.nowMs);
        }
    }

}
