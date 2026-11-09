#ifndef NORTH_ISLAND_UI_ANIMATED_ICON_H
#define NORTH_ISLAND_UI_ANIMATED_ICON_H

#include "ui/Element.h"

#include <cstdint>
#include <string>

namespace ui {

    /// @brief An icon cell that plays its picture strip while the mouse is over it.
    /// @details Original: TAnimateImage. Frames are as high as the element, picture height / width of them. Away from the mouse a strip of three or more frames shows its third, a shorter one its first; under the mouse the cell is filled (HoverColor, alpha 80) and the frames step every 200 ms. Without a picture the cell is empty and takes no clicks unless Active="true" (FIconPicIndex keeps a drag icon). Attributes: Picture, HoverColor, Active. Event: Click.
    class AnimatedIcon : public Element {
        public:
            /// @brief Creates the icon and declares its Click event.
            AnimatedIcon();

            /// @brief Reads the Picture, HoverColor and Active attributes; others go to Element.
            /// @param name Attribute name.
            /// @param value Attribute value text.
            /// @param context Load context for parsing.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves the named picture after all attributes are read.
            /// @param context Load context for picture lookup.
            void finishLoading(const LoadContext& context) override;

            /// @brief Tells whether a point hits the icon; it needs a picture or Active.
            /// @param local Point in element coordinates.
            /// @param pictures Picture library (unused).
            /// @return True when the icon takes a click at that point.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Remembers that the left button went down on the icon.
            /// @param context UI context (unused).
            /// @param button Pressed mouse button.
            /// @param local Point in element coordinates (unused).
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises Click when the left button is let go after a press on the icon.
            /// @param context UI context used to raise the event.
            /// @param button Released mouse button.
            /// @param local Point in element coordinates (unused).
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Replaces the picture and restarts the animation.
            /// @param picture New picture id.
            void setPicture(gfx::PictureId picture);

        protected:
            /// @brief Draws the hover fill and the current frame of the strip.
            /// @details Original: TAnimateImage.DrawControl / RenderWithChildren.
            /// @param context Draw context.
            /// @param screenPosition Top left of the element on screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            std::string m_pictureName;
            gfx::PictureId m_picture;
            // default hover colour #89C1F5 at alpha 80
            core::Color m_hoverColor{0x89, 0xC1, 0xF5, 80};
            bool m_active = false;
            int m_frame = 0;
            std::uint64_t m_frameMs = 0;
            bool m_pressed = false;
    };

}

#endif
