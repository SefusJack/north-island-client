#ifndef NORTH_ISLAND_UI_IMAGE_VIEW_H
#define NORTH_ISLAND_UI_IMAGE_VIEW_H

#include "ui/Element.h"

#include <cstdint>
#include <optional>

namespace ui {

    /// @brief Shows a picture (from the library or a file), a part of it, or an animated vertical strip of frames.
    class ImageView : public Element {
        public:
            /// @brief Reads the ImageView attributes (Picture, Source, SourceRect, HitTest, Frames, FrameInterval).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Looks up the picture and takes its size when the layout gave none.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief With HitTest="true", whether the picture has an opaque pixel under the point.
            /// @param local The point relative to the element.
            /// @param pictures The picture library for the pixel test.
            /// @return True when the element takes the mouse there.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Shows another picture (whole, at the element's size; not animated).
            /// @param picture The new picture.
            void setPicture(gfx::PictureId picture) {
                m_picture = picture;
                m_frames = 1;
                m_frameHeight = 0;
            }

            /// @brief Shows only part of the picture.
            /// @param source The part to show (nullopt: all of it).
            void setSourceRect(const std::optional<core::Rect>& source) { m_sourceRect = source; }

        protected:
            /// @brief Draws the picture (the current animation frame).
            /// @param context The draw context.
            /// @param screenPosition The element's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            gfx::PictureId m_picture;
            std::string m_pictureName;
            std::string m_sourceFile;
            std::optional<core::Rect> m_sourceRect;
            bool m_hitTest = false;
            int m_frames = 1;
            int m_frameHeight = 0;
            int m_frameIntervalMs = 0;
            int m_frame = 0;
            std::uint64_t m_frameStartMs = 0;
    };

}

#endif
