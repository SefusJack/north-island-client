#include "ui/Memo.h"

#include "ui/HintArea.h"
#include "ui/LayoutError.h"

#include <algorithm>
#include <cstdlib>

namespace ui {

    namespace {

        constexpr int kCharWidth = 8; // px width of one text character
        constexpr int kCharHeight = 18; // px height of the text (also the caret height)
        constexpr int kRowHeight = kCharHeight + 2; // px per row: text height plus 2 px spacing
        constexpr std::uint64_t kBlinkMs = 300; // ms between caret blink toggles

    }

    Memo::Memo() {
        setCursor(CursorKind::Hand);
        declareEvent("Typed");
    }

    void Memo::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "FontColor") {
            m_fontColor = LoadContext::parseColor(value);
        } else if (name == "Effect") {
            if (value == "Plain" || value == "Shadow") {
                m_shadow = value == "Shadow";
            } else {
                throw LayoutError("unknown memo effect \"" + value + "\"");
            }
        } else if (name == "MaxChars") {
            m_maxChars = static_cast<std::size_t>(std::max(0, LoadContext::parseInt(value)));
        } else if (name == "ReadOnly") {
            m_readOnly = LoadContext::parseBool(value);
        } else if (name == "Caret") {
            m_caretName = value;
        } else if (name == "JoinRows") {
            m_joinRows = LoadContext::parseBool(value);
        } else {
            Element::applyAttribute(name, value, context);
        }
    }

    void Memo::finishLoading(const LoadContext& context) {
        m_caretPicture = context.picture(m_caretName);
        m_emoteAt = context.emoteAt;
        if (m_emoteAt) {
            for (int emote = 1; emote <= kEmotes; ++emote) {
                m_emotePictures[static_cast<std::size_t>(emote)] = context.picture("icon_expre_" + std::to_string(emote));
            }
        }
        Element::finishLoading(context);
    }

    bool Memo::hitTest(core::Point local, const gfx::PictureLibrary& pictures) const {
        (void)pictures;
        return local.x >= 0 && local.y >= 0 && local.x < bounds().width && local.y < bounds().height;
    }

    int Memo::lineChars() const { return std::max(1, (bounds().width - kCharWidth) / kCharWidth); }

    int Memo::visibleRows() const { return std::max(1, bounds().height / kRowHeight); }

    void Memo::setText(const std::string& text) {
        m_text = text;
        m_topLine = 0;
        m_caretRow = 0;
        m_caretCol = 0;
        if (m_text.empty()) {
            m_lines = {std::string()};
            return;
        }
        layout(false);
    }

    void Memo::layout(bool keepCaret) {
        std::string rest = m_text;
        const bool endsWithReturn = keepCaret && !rest.empty() && rest.back() == '\r';
        const auto width = static_cast<std::size_t>(lineChars());
        m_lines.clear();
        while (!rest.empty()) {
            const std::size_t found = rest.find('\r');
            std::size_t cut = found == std::string::npos ? 0 : found + 1;
            std::string line;
            if (cut > width) {
                line = rest.substr(0, width);
                cut = width;
                if (rest.size() > width && rest[width] == '\r') {
                    ++cut;
                }
            } else if (cut == 0) {
                cut = std::min(rest.size(), width);
                line = rest.substr(0, cut);
            } else {
                line = rest.substr(0, cut - 1);
            }
            rest.erase(0, cut);
            m_lines.push_back(line);
        }
        if (endsWithReturn) {
            m_lines.emplace_back();
        }
    }

    const std::string& Memo::caretLine() const {
        static const std::string kNone;
        const auto row = static_cast<std::size_t>(m_caretRow + m_topLine);
        return m_caretRow + m_topLine >= 0 && row < m_lines.size() ? m_lines[row] : kNone;
    }

    std::size_t Memo::caretOffset() const {
        std::size_t offset = 0;
        for (int row = 0; row < m_caretRow + m_topLine && row < static_cast<int>(m_lines.size()); ++row) {
            offset += m_lines[static_cast<std::size_t>(row)].size();
            if (m_text.size() > offset && m_text[offset] == '\r') {
                ++offset;
            }
        }
        return offset + static_cast<std::size_t>(m_caretCol);
    }

    bool Memo::isFull() const {
        if (m_maxChars == 0) {
            return false;
        }
        std::size_t total = 0;
        for (const auto& line : m_lines) {
            total += line.size();
        }
        return total >= m_maxChars;
    }

    void Memo::insert(char c) {
        const auto u = static_cast<unsigned char>(c);
        // 0x20..0x7E: only printable ASCII characters are typed
        if (u < 0x20 || u >= 0x7F || isFull()) {
            return;
        }
        const std::size_t offset = std::min(caretOffset(), m_text.size());
        if (offset < m_text.size()) {
            if (!m_insertMode) {
                m_text.erase(offset, 1);
            }
            m_text.insert(offset, 1, c);
        } else {
            m_text += c;
        }
        ++m_caretCol;
        if (m_caretCol > lineChars()) {
            ++m_caretRow;
            if (m_caretRow > visibleRows() - 1) {
                m_caretRow = visibleRows() - 1;
                ++m_topLine;
            }
            m_caretCol = 1;
        }
        layout(true);
    }

    void Memo::type(const std::string& text) {
        if (m_readOnly) {
            return;
        }
        for (char c : text) {
            insert(c);
        }
    }

    void Memo::onTextInput(UiContext& context, const std::string& text) {
        type(text);
        if (!m_readOnly) {
            raise(context, "Typed");
        }
    }

    void Memo::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        (void)button;
        (void)local;
        m_caretBlinkMs = context.nowMs;
        m_caretVisible = true;
    }

    void Memo::scrollDown() {
        if (static_cast<int>(m_lines.size()) - visibleRows() - 1 > m_topLine) {
            ++m_topLine;
            m_caretRow = std::max(0, m_caretRow - 1);
        }
    }

    void Memo::scrollUp() {
        if (m_topLine > 0) {
            --m_topLine;
            m_caretRow = std::min(m_caretRow + 1, visibleRows());
        }
    }

    void Memo::pageDown() {
        const int count = static_cast<int>(m_lines.size());
        if (count > visibleRows()) {
            m_topLine = std::min(m_topLine + visibleRows(), count - visibleRows());
        }
    }

    void Memo::pageUp() {
        if (static_cast<int>(m_lines.size()) > visibleRows()) {
            m_topLine = std::max(0, m_topLine - visibleRows());
        }
    }

    void Memo::onMouseWheel(UiContext& context, int steps) {
        (void)context;
        for (int i = 0; i < std::abs(steps); ++i) {
            steps > 0 ? scrollUp() : scrollDown();
        }
    }

    void Memo::onKeyDown(UiContext& context, const KeyEvent& key) {
        (void)context;
        if (m_readOnly && (key.key == Key::Backspace || key.key == Key::Delete || key.key == Key::Enter)) {
            return;
        }
        const auto lineSize = [this] { return static_cast<int>(caretLine().size()); };
        switch (key.key) {
            case Key::Backspace: { // delete the character before the caret
                const std::size_t offset = caretOffset();
                if (offset == 0 || offset > m_text.size()) {
                    return;
                }
                std::size_t size = m_caretCol == 0 || m_caretCol > lineSize() ? 0 : 1;
                if (size == 0 && m_text[offset - 1] == '\r') {
                    size = 1;
                }
                m_text.erase(offset - size, size);
                if (m_caretCol < 1) {
                    if (m_caretRow + m_topLine != 0 && !m_lines.empty()) {
                        m_caretCol = static_cast<int>(m_lines[static_cast<std::size_t>(m_caretRow + m_topLine - 1)].size());
                    }
                    if (--m_caretRow < 0) {
                        m_topLine = std::max(0, m_topLine - 1);
                        m_caretRow = 0;
                    }
                } else {
                    m_caretCol -= static_cast<int>(size);
                }
                layout(true);
                break;
            }
            case Key::Delete: { // delete the character at the caret
                const std::size_t offset = caretOffset();
                if (m_lines.empty() || offset >= m_text.size()) {
                    return;
                }
                m_text.erase(offset, 1);
                layout(true);
                break;
            }
            case Key::Left: // caret one column left, or to the end of the row above
                if (m_caretCol == 0 || m_caretCol > lineSize()) {
                    if (m_caretRow - 1 < 0) {
                        return;
                    }
                    --m_caretRow;
                    m_caretCol = lineSize();
                    return;
                }
                --m_caretCol;
                break;
            case Key::Right: // caret one column right, or to the start of the row below
                if (m_caretCol >= lineSize()) {
                    if (static_cast<int>(m_lines.size()) - m_topLine - 1 < m_caretRow + 1) {
                        return;
                    }
                    ++m_caretRow;
                    m_caretCol = 0;
                    return;
                }
                ++m_caretCol;
                break;
            case Key::Insert: // toggle insert / overwrite mode
                m_insertMode = !m_insertMode;
                break;
            case Key::Home: // caret to the start of the row
                m_caretCol = 0;
                break;
            case Key::End: // caret to the end of the row
                m_caretCol = lineSize();
                break;
            case Key::Enter: // end the row with a carriage return and move the caret to the next row
                if (isFull()) {
                    return;
                }
                if (m_insertMode) {
                    const std::size_t offset = caretOffset() + (m_caretCol > lineSize() ? 1 : 0);
                    m_text.insert(std::min(offset, m_text.size()), 1, '\r');
                    layout(true);
                }
                m_caretCol = 0;
                if (++m_caretRow > visibleRows() - 1) {
                    ++m_topLine;
                    m_caretRow = visibleRows() - 1;
                }
                break;
            case Key::Up: // caret one row up, scrolling at the top
                if (--m_caretRow < 0) {
                    m_caretRow = 0;
                    m_topLine = std::max(0, m_topLine - 1);
                }
                m_caretCol = std::min(m_caretCol, lineSize());
                break;
            case Key::Down: { // caret one row down, scrolling at the bottom
                const int count = static_cast<int>(m_lines.size());
                ++m_caretRow;
                if (count - 1 < m_caretRow + m_topLine) {
                    m_caretRow = std::max(0, count - 1 - m_topLine);
                }
                if (m_caretRow > visibleRows() - 1) {
                    m_caretRow = visibleRows() - 1;
                    ++m_topLine;
                    if (count < m_topLine + visibleRows() - 1) {
                        m_topLine = count - visibleRows() + 1;
                    }
                }
                m_caretCol = std::min(m_caretCol, lineSize());
                break;
            }
            default: // other keys do nothing
                break;
        }
    }

    std::string Memo::drawRow(DrawContext& context, const std::string& row, core::Point at) const {
        constexpr int kCodeLength = 2; // characters of an emote code
        constexpr int kIconSize = 16; // px size of an emote icon (square frame)
        constexpr std::uint64_t kIconFrameMs = 210; // ms each emote animation frame shows
        if (!m_emoteAt) {
            return row;
        }
        std::string shown = row;
        for (std::size_t column = 0; column < row.size();) {
            if (static_cast<unsigned char>(row[column]) >= 0x80) { // 0x80 and over: a two-byte character
                column += kCodeLength;
                continue;
            }
            const int emote = m_emoteAt(row, column);
            if (emote < 1 || emote > kEmotes) {
                ++column;
                continue;
            }
            const gfx::PictureId picture = m_emotePictures[static_cast<std::size_t>(emote)];
            if (picture.isValid()) {
                const int frame = static_cast<int>(context.nowMs / kIconFrameMs % 2); // the emote has 2 frames
                context.canvas
                    .drawPicture(picture, {at.x + static_cast<int>(column) * kCharWidth, at.y}, core::Rect{0, frame * kIconSize, kIconSize, kIconSize});
            }
            shown.replace(column, kCodeLength, std::string(kCodeLength, ' '));
            column += kCodeLength;
        }
        return shown;
    }

    void Memo::draw(DrawContext& context, core::Point screenPosition) {
        const gfx::TextStyle style{m_fontColor, m_shadow ? gfx::TextEffect::Shadow : gfx::TextEffect::Plain, std::nullopt};
        std::string joined;
        for (int row = 0; row < visibleRows(); ++row) {
            const int index = m_topLine + row;
            if (index < 0 || index >= static_cast<int>(m_lines.size())) {
                break;
            }
            const std::string& line = m_lines[static_cast<std::size_t>(index)];
            joined += line;
            if (!m_joinRows && !line.empty()) {
                const core::Point at{screenPosition.x, screenPosition.y + kRowHeight * row};
                context.canvas.drawText(at, drawRow(context, line, at), style, bounds().width);
            }
        }
        if (m_joinRows && !joined.empty()) {
            context.canvas.drawText(screenPosition, HintArea::wrap(joined, bounds().width / kCharWidth - 1), style, bounds().width - kCharWidth, kCharHeight);
        }
        if (context.focusedId != id()) {
            return;
        }
        if (m_caretCol * kCharWidth > bounds().width) {
            m_caretCol = 1;
        }
        if (context.nowMs > m_caretBlinkMs + kBlinkMs) {
            m_caretBlinkMs = context.nowMs;
            m_caretVisible = !m_caretVisible;
        }
        if (m_caretVisible && !m_readOnly) {
            context.canvas.drawPicture(
                m_caretPicture,
                {screenPosition.x + m_caretCol * kCharWidth - 1, screenPosition.y + m_caretRow * kRowHeight},
                core::Rect{0, 0, 3, kCharHeight}
            ); // the caret picture is 3 pixels wide
        }
    }

}
