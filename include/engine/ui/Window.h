#ifndef NORTH_ISLAND_UI_WINDOW_H
#define NORTH_ISLAND_UI_WINDOW_H

#include "ui/Element.h"

namespace ui {

    /// @brief A movable form with a background picture and controls on it, snapping to the screen edges.
    /// @details Original: TSe_FixedForm (TSe_Form with ScreenFit).
    class Window : public Element {
        public:
            /// @brief Creates the window and declares its events.
            Window();

            /// @brief Reads a layout attribute (Picture, ScreenFit, AbovePopups).
            /// @param name Attribute name.
            /// @param value Attribute text.
            /// @param context Load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves the background picture and takes its size when none was given.
            /// @param context Load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Shows another background (the size changes with it).
            /// @param name Picture name.
            /// @param pictures Picture library to look it up in.
            void setPicture(const std::string& name, gfx::PictureLibrary& pictures);

            /// @brief Pulls a ScreenFit window back onto the screen (every frame; for a window moved from code at once).
            /// @details Original: TSe_Form.RenderWithChildren.
            void fitToScreen();

            /// @brief Fits or snaps the window to the view each frame.
            /// @param context UI context.
            void update(UiContext& context) override;

            /// @brief Tells whether a point is inside the window.
            /// @param local Point in the window's coordinates.
            /// @param pictures Unused.
            /// @return True when inside.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Gives the hint, shown only over the title strip (the top 24 px).
            /// @param local Mouse in the window's coordinates.
            /// @return The hint, or an empty string.
            const std::string& hintAt(core::Point local) const override;

            /// @brief Starts a drag and raises MouseDown for the left button.
            /// @param context UI context.
            /// @param button Button pressed.
            /// @param local Mouse in the window's coordinates.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Ends the drag and raises MouseUp and Click for the left button.
            /// @param context UI context.
            /// @param button Button released.
            /// @param local Mouse in the window's coordinates.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Ends the drag when the mouse is released elsewhere.
            /// @param context UI context.
            void onCaptureReleased(UiContext& context) override;

            /// @brief Moves the window with the mouse and snaps it to the edges.
            /// @details Original: TSe_FixedForm.HandleMouseMove; TSe_Form.HandleMouseMove keeps the form above the bottom only.
            /// @param context UI context.
            /// @param local Mouse in the window's coordinates.
            void onMouseDrag(UiContext& context, core::Point local) override;

            /// @brief Raises DoubleClick (before the press of its second click).
            /// @param context UI context.
            /// @param local Mouse in the window's coordinates.
            void onDoubleClick(UiContext& context, core::Point local) override;

            /// @brief Tells that a press brings the window on top of the other layouts.
            /// @return Always true.
            bool raisesOnPress() const override { return true; }

            /// @brief Tells whether a press brings the window over the topmost popups too.
            /// @return True with AbovePopups.
            bool raisesAbovePopups() const override { return m_abovePopups; }

            /// @brief Tells whether the layout gave the Escape event a handler.
            /// @return True when it did.
            bool closesOnEscape() const { return hasHandler("Escape"); }

            /// @brief Raises the Escape event (the window closes itself).
            /// @param context UI context.
            void raiseEscape(UiContext& context) const { raise(context, "Escape"); }

        protected:
            /// @brief Draws the background picture.
            /// @param context Draw context.
            /// @param screenPosition Screen position of the window.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief Pulls the window to a screen edge when it is near one.
            /// @details Original: TSe_FixedForm.RenderWithChildren while FDragging; a press without a move included.
            void snapToEdges();

            std::string m_pictureName;
            gfx::PictureId m_picture;
            core::Point m_grab;
            bool m_pressed = false;
            bool m_screenFit = false;
            // the game's 800 x 600 screen
            core::Rect m_view{0, 0, 800, 600};
            bool m_abovePopups = false;
    };

}

#endif
