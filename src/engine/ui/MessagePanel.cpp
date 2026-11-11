#include "ui/MessagePanel.h"

#include "ui/TextWrap.h"

#include <algorithm>

namespace ui {

    namespace {

        const core::Color kRedColor = core::Color::fromRgb565(0xF800); // RGB565 0xF800: pure red, for #R text

    }

    void MessagePanel::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "Skin") {
            m_skinName = value;
        } else if (name == "SkinSource") {
            m_skinSource = LoadContext::parseRect(value);
        } else if (name == "Borders" || name == "Margins") {
            const std::vector<int> numbers = LoadContext::parseInts(value, 4);
            std::array<int, 4>& target = name == "Borders" ? m_borders : m_margins;
            std::copy(numbers.begin(), numbers.end(), target.begin());
        } else if (name == "WrapColumns") {
            m_wrapColumns = static_cast<std::size_t>(std::max(1, LoadContext::parseInt(value)));
        } else if (name == "MaxLines") {
            m_maxLines = static_cast<std::size_t>(std::max(1, LoadContext::parseInt(value)));
        } else if (name == "LineHeight") {
            m_lineHeight = LoadContext::parseInt(value);
        } else if (name == "LineSpacing") {
            m_lineSpacing = LoadContext::parseInt(value);
        } else if (name == "FontColor") {
            m_fontColor = LoadContext::parseColor(value);
        } else if (name == "Queued") {
            m_queued = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void MessagePanel::finishLoading(const LoadContext& context) {
        const gfx::PictureId picture = context.picture(m_skinName);
        if (picture.isValid()) {
            const gfx::Picture& data = context.pictureData(picture);
            const core::Rect source = m_skinSource.value_or(core::Rect{0, 0, data.width(), data.height()});
            m_skin = TiledFrame(picture, source, m_borders[0], m_borders[1], m_borders[2], m_borders[3]);
        }
        Element::finishLoading(context);
    }

    void MessagePanel::show(const std::string& text, int durationMs, std::uint64_t nowMs) {
        const std::uint64_t expires = nowMs + static_cast<std::uint64_t>(std::max(0, durationMs));
        const MessageTags tags(text, std::string(), std::string());
        const std::string& plain = tags.text();
        const std::size_t lineBreak = plain.find_first_of("\r\n");
        if (!m_queued || (lineBreak != std::string::npos && lineBreak + 1 < plain.size())) {
            m_lines.clear();
        }
        const std::vector<std::string> lines = wrapText(plain, m_wrapColumns);
        for (std::size_t row = 0; row < lines.size(); ++row) {
            m_lines.push_back({lines[row], expires, tags.tagged() ? tags.segments(lines, row) : std::vector<MessageTags::Segment>()});
        }
        while (m_lines.size() > m_maxLines) {
            m_lines.erase(m_lines.begin());
        }
        m_closeAtMs = expires;
        setVisible(!m_lines.empty());
    }

    void MessagePanel::append(const std::string& text, int durationMs, std::uint64_t nowMs, std::size_t mostLines) {
        const std::uint64_t expires = nowMs + static_cast<std::uint64_t>(std::max(0, durationMs));
        const MessageTags tags(text, std::string(), std::string());
        const std::vector<std::string> lines = wrapText(tags.text(), m_wrapColumns);
        for (std::size_t row = 0; row < lines.size(); ++row) {
            m_lines.push_back({lines[row], expires, tags.tagged() ? tags.segments(lines, row) : std::vector<MessageTags::Segment>()});
        }
        while (m_lines.size() > std::max(mostLines, m_maxLines)) {
            m_lines.erase(m_lines.begin());
        }
        m_closeAtMs = std::max(m_closeAtMs, expires);
        setVisible(!m_lines.empty());
    }

    void MessagePanel::close() {
        m_lines.clear();
        setVisible(false);
    }

    void MessagePanel::keepBelow(int minTop) {
        if (isVisible() && bounds().y < minTop) {
            setPosition({bounds().x, minTop});
        }
    }

    void MessagePanel::update(UiContext& context) {
        if (!isVisible()) {
            return;
        }
        if (context.nowMs >= m_closeAtMs) {
            close();
            return;
        }
        m_lines.erase(std::remove_if(m_lines.begin(), m_lines.end(), [&](const Line& line) { return context.nowMs >= line.expiresMs; }), m_lines.end());
        if (m_lines.empty()) {
            close();
        }
    }

    void MessagePanel::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        (void)context;
        (void)button;
        (void)local;
        close();
    }

    core::Rect MessagePanel::panelArea(core::Point screenPosition) const {
        std::size_t longest = 0;
        for (const Line& line : m_lines) {
            longest = std::max(longest, line.text.size());
        }
        // 8: pixel width of a character; width gets 10 extra pixels, height 4 + 5 (CalcMsgPanelSize)
        // m_margins[0..3] = left, top, right, bottom
        const int width = static_cast<int>(longest) * 8 + m_margins[0] + m_margins[2] + 10;
        const int height = static_cast<int>(m_lines.size()) * m_lineHeight + m_margins[1] + m_margins[3] + 4 + 5;
        return {screenPosition.x + (bounds().width - width) / 2, screenPosition.y, width, height};
    }

    bool MessagePanel::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return isVisible() && panelArea({0, 0}).contains(local);
    }

    void MessagePanel::draw(DrawContext& context, core::Point screenPosition) {
        if (m_lines.empty()) {
            return;
        }
        const core::Rect area = panelArea(screenPosition);
        m_skin.draw(context.canvas, area);
        for (std::size_t row = 0; row < m_lines.size(); ++row) {
            const std::string& text = m_lines[row].text;
            // 8: pixel width of a character; the text sits 3 pixels left of centre and 2 pixels below the top margin
            const int x = area.x + (area.width - static_cast<int>(text.size()) * 8) / 2 - 3;
            const int y = area.y + m_margins[1] + 2 + (m_lineHeight + m_lineSpacing) * static_cast<int>(row);
            if (m_lines[row].pieces.empty()) {
                context.canvas
                    .drawText({x, y}, text, {m_fontColor, gfx::TextEffect::Shadow, std::nullopt}, area.width, 20); // 20: line height passed to drawText
                continue;
            }
            for (const MessageTags::Segment& piece : m_lines[row].pieces) {
                // 8: pixel width of a character; 20: line height passed to drawText
                context.canvas.drawText(
                    {x + static_cast<int>(piece.from) * 8, y},
                    text.substr(piece.from, piece.length),
                    {piece.red ? kRedColor : m_fontColor, gfx::TextEffect::Shadow, std::nullopt},
                    area.width,
                    20
                );
            }
        }
    }

}
