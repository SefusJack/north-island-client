#ifndef NORTH_ISLAND_UI_HINT_AREA_H
#define NORTH_ISLAND_UI_HINT_AREA_H

#include "core/Color.h"
#include "ui/Element.h"

#include <string>

namespace ui {

    /// @brief An invisible area that shows a wrapped tooltip box over everything while the mouse is on it.
    class HintArea : public Element {
        public:
            /// @brief Reads the HintArea attributes (Hint, LineChars, LineHeight, FontColor, Background, BackgroundAlpha, BorderColor).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Cuts text into lines of exactly lineChars characters, hyphenating words broken inside.
            /// @details Original: WrapTextLines.
            /// @param text The text to cut.
            /// @param lineChars The line width in characters.
            /// @return The text with the lines joined without separators.
            static std::string wrap(const std::string& text, int lineChars);

        protected:
            /// @brief Draws the tooltip box over everything while the mouse is in the area.
            /// @param context The draw context.
            /// @param screenPosition The area's top left on the screen.
            void drawOverlay(DrawContext& context, core::Point screenPosition) override;

        private:
            std::string m_hint;
            int m_lineChars = 20; // characters per line before the hint wraps
            int m_lineHeight = 20; // line spacing, px
            core::Color m_fontColor{255, 255, 255, 255}; // white
            core::Color m_background{61, 138, 249, 200}; // light blue, alpha 200 (semi-transparent)
            core::Color m_border{0, 0, 255, 255}; // blue
    };

}

#endif
