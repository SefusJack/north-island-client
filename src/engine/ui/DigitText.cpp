#include "ui/DigitText.h"

#include <algorithm>

namespace ui {

    void DigitText::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Picture") {
            m_pictureName = value;
        } else if (name == "Rows") {
            m_rows = std::max(1, LoadContext::parseInt(value));
        } else if (name == "Glyphs") {
            m_glyphs = value;
        } else if (name == "Spacing") {
            m_spacing = LoadContext::parseInt(value);
        } else if (name == "Text") {
            m_text = value;
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void DigitText::finishLoading(const LoadContext& context) {
        m_picture = context.picture(m_pictureName);
        if (m_picture.isValid()) {
            const gfx::Picture& picture = context.pictureData(m_picture);
            m_glyphWidth = picture.width();
            m_glyphHeight = picture.height() / m_rows;
        }
        Element::finishLoading(context);
    }

    void DigitText::draw(DrawContext& context, core::Point screenPosition) {
        int x = screenPosition.x;
        for (char c : m_text) {
            const std::size_t row = m_glyphs.find(c);
            if (row == std::string::npos) {
                continue;
            }
            context.canvas.drawPicture(m_picture, {x, screenPosition.y}, core::Rect{0, static_cast<int>(row) * m_glyphHeight, m_glyphWidth, m_glyphHeight});
            x += m_glyphWidth + m_spacing;
        }
    }

}
