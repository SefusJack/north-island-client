#ifndef NORTH_ISLAND_UI_PANEL_H
#define NORTH_ISLAND_UI_PANEL_H

#include "ui/Element.h"
#include "ui/TiledFrame.h"

namespace ui {

    /// @brief A container, optionally drawn with a tiled skin.
    class Panel : public Element {
        public:
            /// @brief Reads the Panel attributes (Skin, SkinSource, Borders, HitTest, ClipChildren).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Builds the tiled skin from the skin picture.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Whether the panel takes the mouse: everywhere with HitTest, else where its skin is opaque.
            /// @param local The point relative to the panel.
            /// @param pictures The picture library for the pixel test.
            /// @return True when the panel takes the mouse there.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Whether the children are clipped to the panel (ClipChildren).
            /// @return True when clipped.
            bool clipsChildren() const override { return m_clipChildren; }

            /// @brief Another picture in the same part and borders (a talk box's skin).
            /// @param picture The new skin picture.
            /// @param pictures The picture library to read its size from.
            void setSkinPicture(gfx::PictureId picture, const gfx::PictureLibrary& pictures);

            /// @brief The part of the skin picture drawn.
            /// @param source The part (nullopt: the whole picture).
            void setSkinSource(std::optional<core::Rect> source) { m_skinSource = source; }

        protected:
            /// @brief Draws the tiled skin over the panel's bounds.
            /// @param context The draw context.
            /// @param screenPosition The panel's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

            /// @brief The tiled skin.
            /// @return The skin frame.
            const TiledFrame& skin() const { return m_skin; }

        private:
            TiledFrame m_skin;
            std::string m_skinName;
            std::optional<core::Rect> m_skinSource;
            int m_borders[4] = {6, 6, 6, 6};
            gfx::PictureId m_skinPicture;
            bool m_hitTest = false;
            bool m_clipChildren = false;
    };

}

#endif
