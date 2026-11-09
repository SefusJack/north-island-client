#include "ui/HintArea.h"

#include "ui/WrappedTooltip.h"

#include <algorithm>
#include <cctype>

namespace ui {

    namespace {

        /// @brief Whether a character is an ASCII letter.
        /// @param c The character.
        /// @return True for a letter.
        bool isLetter(char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0; }

    }

    void HintArea::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Hint") {
            m_hint = value;
        } else if (name == "LineChars") {
            m_lineChars = std::max(2, LoadContext::parseInt(value)); // at least 2 characters per line (wrap looks at width - 2)
        } else if (name == "LineHeight") {
            m_lineHeight = LoadContext::parseInt(value);
        } else if (name == "FontColor") {
            m_fontColor = LoadContext::parseColor(value);
        } else if (name == "Background") {
            const std::uint8_t alpha = m_background.a;
            m_background = LoadContext::parseColor(value);
            m_background.a = alpha;
        } else if (name == "BackgroundAlpha") {
            m_background.a = static_cast<std::uint8_t>(std::clamp(LoadContext::parseInt(value), 0, 255)); // alpha range
        } else if (name == "BorderColor") {
            m_border = LoadContext::parseColor(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    std::string HintArea::wrap(const std::string& text, int lineChars) {
        const std::size_t width = static_cast<std::size_t>(lineChars);
        std::string rest = text;
        std::string result;
        while (!rest.empty()) {
            if (rest.size() <= width) {
                result += rest;
                break;
            }
            const char beforeLast = rest[width - 2]; // the second last character of the line
            if (isLetter(beforeLast) && isLetter(rest[width - 1]) && isLetter(rest[width])) {
                result += rest.substr(0, width - 1) + '-';
                rest.erase(0, width - 1);
            } else if (!isLetter(beforeLast)) {
                result += rest.substr(0, width - 1) + ' ';
                rest.erase(0, width - 1);
            } else {
                result += rest.substr(0, width);
                rest.erase(0, width);
            }
        }
        return result;
    }

    void HintArea::drawOverlay(DrawContext& context, core::Point screenPosition) {
        const core::Rect area{screenPosition.x, screenPosition.y, bounds().width, bounds().height};
        if (m_hint.empty() || !area.contains(context.mouse)) {
            return;
        }
        // the box starts 2 pixels right of the area
        drawWrappedTooltip(context.canvas, {area.right() + 2, area.y}, m_hint, m_lineChars, m_lineHeight, m_fontColor, m_background, m_border);
    }

}
