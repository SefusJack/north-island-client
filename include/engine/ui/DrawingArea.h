#ifndef NORTH_ISLAND_UI_DRAWING_AREA_H
#define NORTH_ISLAND_UI_DRAWING_AREA_H

#include "ui/Element.h"

#include <cstdint>
#include <functional>

namespace ui {

    /// @brief A place in a layout that its view model paints itself (characters, maps).
    /// @details The layout gives the position, the view model a painter. The mouse passes through unless HitTest="true"; then the left button raises MouseDown, MouseDrag (while held) and MouseUp with the point relative to the area; the right button raises RightMouseDown and RightMouseUp; a double click raises DoubleClick (before the MouseDown of its second press), and MouseWheel (index: the steps, positive up) while it holds the focus (the area the mouse went down on). A drag its view model starts (UiContext.drag) raises Drop where the button comes up (index: the dragged index; target, targetIndex).
    class DrawingArea : public Element {
        public:
            using Painter = std::function<void(gfx::Canvas& canvas, core::Point screenPosition, std::uint64_t nowMs)>;

            /// @brief Creates the area and declares its mouse events.
            DrawingArea();

            /// @brief Sets the view model's painter.
            /// @param painter Function that paints the area.
            void setPainter(Painter painter) { m_painter = std::move(painter); }

            /// @brief Turns mouse hit testing on or off.
            /// @param hitTest True to let the area take the mouse.
            void setHitTest(bool hitTest) { m_hitTest = hitTest; }

            /// @brief Sets the view model's own test of which points of the area take the mouse (a picture's opaque pixels as the game maps them).
            /// @param tester Returns true for a local point that takes the mouse; for a hit-tested area a point of its rectangle takes the mouse only when this says so.
            void setHitTester(std::function<bool(core::Point local)> tester) { m_hitTester = std::move(tester); }

            /// @brief Reads the HitTest attribute; others go to Element.
            /// @param name Attribute name.
            /// @param value Attribute value text.
            /// @param context Load context for parsing.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Tells whether a point takes the mouse (HitTest on, inside the bounds and passing the tester).
            /// @param local Point in element coordinates.
            /// @param pictures Picture library (unused).
            /// @return True when the area takes the mouse there.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Raises MouseDown (left) or RightMouseDown (right).
            /// @param context UI context used to raise events.
            /// @param button Pressed mouse button.
            /// @param local Point in element coordinates.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises MouseDrag while the button is held.
            /// @param context UI context used to raise the event.
            /// @param local Point in element coordinates.
            void onMouseDrag(UiContext& context, core::Point local) override;

            /// @brief Raises MouseUp (left) or RightMouseUp (right).
            /// @param context UI context used to raise events.
            /// @param button Released mouse button.
            /// @param local Point in element coordinates.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises DoubleClick.
            /// @param context UI context used to raise the event.
            /// @param local Point in element coordinates.
            void onDoubleClick(UiContext& context, core::Point local) override;

            /// @brief Raises MouseWheel with the steps as index.
            /// @param context UI context used to raise the event.
            /// @param steps Wheel steps, positive up.
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Raises MouseUp when the button came up outside the area.
            /// @param context UI context used to raise the event.
            void onCaptureReleased(UiContext& context) override;

            /// @brief Raises Drop where a drag started by the view model comes up.
            /// @param context UI context used to raise the event.
            /// @param sourceIndex The dragged index.
            /// @param target Element under the drop, if any.
            /// @param targetIndex Cell index in the target.
            /// @param screenPoint Drop point on screen.
            /// @param grab Where the mouse held the dragged icon.
            void onDropped(
                UiContext& context,
                int sourceIndex,
                std::optional<std::reference_wrapper<const Element>> target,
                int targetIndex,
                core::Point screenPoint,
                core::Point grab
            ) override;

        protected:
            /// @brief Calls the painter, if set.
            /// @param context Draw context.
            /// @param screenPosition Top left of the element on screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            Painter m_painter;
            std::function<bool(core::Point local)> m_hitTester;
            bool m_hitTest = false;
            core::Point m_lastMouse;
    };

}

#endif
