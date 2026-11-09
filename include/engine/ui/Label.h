#ifndef NORTH_ISLAND_UI_LABEL_H
#define NORTH_ISLAND_UI_LABEL_H

#include "ui/Element.h"

#include <string>

namespace ui {

    /// @brief Text in the game's bitmap font; the mouse passes through.
    class Label : public Element {
        public:
            /// @brief Reads the Label attributes (Text, FontColor, Effect, LineHeight, Align, WordWrap).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief How a line shorter than the element's width is placed in it.
            enum class Align { Left, Center, Right };

            /// @brief Sets the label text.
            /// @param text The new text.
            void setText(const std::string& text) { m_text = text; }

            /// @brief The label text.
            /// @return The text.
            const std::string& text() const { return m_text; }

        protected:
            /// @brief Draws the text.
            /// @param context The draw context.
            /// @param screenPosition The label's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            std::string m_text;
            gfx::TextStyle m_style;
            int m_lineHeight = 15; // line spacing, px (the small font's height)
            Align m_align = Align::Left;
            bool m_wordWrap = false; // break lines at spaces; needs a width (without it, a width breaks lines anywhere)
    };

}

#endif
