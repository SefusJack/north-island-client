#include "ui/Label.h"

#include "gfx/TextRenderer.h"
#include "ui/LayoutError.h"

#include <algorithm>

namespace ui {

    void Label::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Text") {
            m_text = value;
        } else if (name == "FontColor") {
            m_style.color = LoadContext::parseColor(value);
        } else if (name == "Effect") {
            m_style.effect = LoadContext::parseEffect(value);
        } else if (name == "Align") {
            if (value == "Left") {
                m_align = Align::Left;
            } else if (value == "Center") {
                m_align = Align::Center;
            } else if (value == "Right") {
                m_align = Align::Right;
            } else {
                throw LayoutError("unknown alignment \"" + value + "\"");
            }
        } else if (name == "LineHeight") {
            m_lineHeight = LoadContext::parseInt(value);
        } else if (name == "WordWrap") {
            m_wordWrap = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void Label::draw(DrawContext& context, core::Point screenPosition) {
        if (m_text.empty()) {
            return;
        }
        if (m_wordWrap && bounds().width > 0) {
            core::Point at = screenPosition;
            for (const std::string& line : gfx::TextRenderer::wrapLines(m_text, m_style.effect, bounds().width)) {
                const int free = std::max(0, bounds().width - gfx::TextRenderer::textWidth(line, m_style.effect));
                const int indent = m_align == Align::Center ? free / 2 : m_align == Align::Right ? free : 0;
                context.canvas.drawText({at.x + indent, at.y}, line, m_style, bounds().width, m_lineHeight);
                at.y += m_lineHeight;
            }
            return;
        }
        const int maxWidth = bounds().width > 0 ? bounds().width : 100000; // no width given: effectively unlimited wrap width in pixels
        core::Point at = screenPosition;
        if (m_align != Align::Left && bounds().width > 0) {
            const int free = std::max(0, bounds().width - gfx::TextRenderer::textWidth(m_text, m_style.effect));
            at.x += m_align == Align::Center ? free / 2 : free;
        }
        context.canvas.drawText(at, m_text, m_style, maxWidth, m_lineHeight);
    }

}
