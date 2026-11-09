#ifndef NORTH_ISLAND_UI_DIGIT_TEXT_H
#define NORTH_ISLAND_UI_DIGIT_TEXT_H

#include "ui/Element.h"

#include <string>

namespace ui {

    /// @brief Numbers drawn from a picture of glyphs stacked vertically (Num_MainInfo, Num_White3_1).
    /// @details Attributes: Picture, Rows (the picture height is split into that many glyphs), Glyphs (which character each row shows, top first), Spacing (added to the glyph width between characters), Text. Characters without a glyph are skipped.
    class DigitText : public Element {
        public:
            /// @brief Reads the Picture, Rows, Glyphs, Spacing and Text attributes; others go to Element.
            /// @param name Attribute name.
            /// @param value Attribute value text.
            /// @param context Load context for parsing.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves the picture and works out the glyph size.
            /// @param context Load context for picture lookup.
            void finishLoading(const LoadContext& context) override;

            /// @brief Sets the text to draw.
            /// @param text New text.
            void setText(const std::string& text) { m_text = text; }

        protected:
            /// @brief Draws the text glyph by glyph.
            /// @param context Draw context.
            /// @param screenPosition Top left of the element on screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            std::string m_pictureName;
            gfx::PictureId m_picture;
            int m_rows = 10; // default glyph rows for the digits 0-9
            std::string m_glyphs = "0123456789";
            int m_spacing = 0;
            int m_glyphWidth = 0;
            int m_glyphHeight = 0;
            std::string m_text;
    };

}

#endif
