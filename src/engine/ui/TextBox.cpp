#include "ui/TextBox.h"

#include "gfx/TextRenderer.h"
#include "ui/LayoutError.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace ui {

    namespace {

        constexpr int kCharWidth = 8; // fixed width of one text character, px
        constexpr int kTextLeft = 4; // left padding of the text inside the box, px
        constexpr std::uint64_t kBlinkMs = 300; // caret blink interval, ms

    }

    TextBox::TextBox() {
        declareEvent("Enter");
        declareEvent("Tab");
        declareEvent("TextChanged");
        declareEvent("Click");
        declareEvent("Up");
        declareEvent("Down");
        declareEvent("WheelUp");
        declareEvent("WheelDown");
        setCursor(CursorKind::Hand);
    }

    void TextBox::applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) {
        if (name == "FontColor") {
            m_fontColor = LoadContext::parseColor(value);
        } else if (name == "MaxLength") {
            m_maxLength = static_cast<std::size_t>(std::max(0, LoadContext::parseInt(value)));
        } else if (name == "Password") {
            m_password = LoadContext::parseBool(value);
        } else if (name == "Align") {
            m_alignRight = value == "Right";
        } else if (name == "ReadOnly") {
            m_readOnly = LoadContext::parseBool(value);
        } else if (name == "AllowPaste") {
            m_allowPaste = LoadContext::parseBool(value);
        } else if (name == "Caret") {
            m_caretName = value;
        } else if (name == "CharFilter") {
            if (value == "Any") {
                m_filter = CharFilter::Any;
            } else if (value == "Digits") {
                m_filter = CharFilter::Digits;
            } else if (value == "Alphanumeric") {
                m_filter = CharFilter::Alphanumeric;
            } else {
                throw LayoutError("unknown CharFilter \"" + value + "\"");
            }
        } else {
            Panel::applyAttribute(name, value, context);
        }
    }

    void TextBox::finishLoading(const LoadContext& context) {
        m_caretPicture = context.picture(m_caretName);
        Panel::finishLoading(context);
    }

    // 2: right padding of the text area in pixels
    int TextBox::visibleChars() const { return std::max(1, (bounds().width - kTextLeft - 2) / kCharWidth); }

    void TextBox::keepCaretVisible() {
        const auto visible = static_cast<std::size_t>(visibleChars());
        if (m_caret < m_scroll) {
            m_scroll = m_caret;
        } else if (m_caret > m_scroll + visible) {
            m_scroll = m_caret - visible;
        }
    }

    int TextBox::scrollRightColumn(int shift) const {
        const int visible = visibleChars();
        if (shift > 0) {
            return visible - 4; // caret column 4 from the right edge after a Right key
        }
        // columns 4 or 3 from the right edge, 6 characters of lookahead (original EditorScrollRightCaretCol)
        return 1 > static_cast<int>(m_text.size()) - visible + 6 ? visible - 4 : visible - 3;
    }

    int TextBox::scrollLeftColumn() const {
        // near the start (within 4 characters) the caret keeps its column
        if (m_caret <= 4) {
            return static_cast<int>(m_caret);
        }
        // 0x7F: below it a single byte character (1 column), else a double byte one (2 columns)
        return static_cast<unsigned char>(m_text[m_caret - 1]) < 0x7F ? 1 : 2;
    }

    void TextBox::setColumn(int column) { m_scroll = static_cast<std::size_t>(std::max<long long>(0, static_cast<long long>(m_caret) - column)); }

    void TextBox::setText(const std::string& text) {
        m_text = text;
        m_caret = m_text.size();
        m_scroll = 0;
        keepCaretVisible();
    }

    void TextBox::setTextFromStart(const std::string& text) {
        m_text = text;
        m_caret = 0;
        m_scroll = 0;
    }

    bool TextBox::accepts(char c) const {
        const auto u = static_cast<unsigned char>(c);
        // control characters (below space) and DEL are refused
        if (u < 0x20 || u == 0x7F) {
            return false;
        }
        switch (m_filter) {
            case CharFilter::Digits: // only digits
                return std::isdigit(u) != 0;
            case CharFilter::Alphanumeric: // letters and digits
                return std::isalnum(u) != 0;
            case CharFilter::Any: // everything printable
                break;
        }
        return true;
    }

    void TextBox::insert(UiContext& context, const std::string& text) {
        std::vector<std::string> characters;
        for (std::size_t i = 0; i < text.size();) {
            // 128: a lead byte of a double byte character
            const std::size_t size = static_cast<unsigned char>(text[i]) >= 128 && i + 1 < text.size() ? 2 : 1;
            std::string character;
            for (std::size_t k = 0; k < size; ++k) {
                if (accepts(text[i + k])) {
                    character += text[i + k];
                }
            }
            i += size;
            if (!character.empty()) {
                characters.push_back(character);
            }
        }
        if (characters.empty()) {
            return;
        }
        if (m_filter == CharFilter::Digits && m_text == "0") {
            m_text.clear();
            m_caret = 0;
        }
        bool changed = false;
        for (const std::string& character : characters) {
            if (m_maxLength > 0 && m_text.size() + character.size() > m_maxLength) {
                continue;
            }
            const int column = static_cast<int>(m_caret - std::min(m_scroll, m_caret));
            m_text.insert(m_caret, character);
            m_caret += character.size();
            changed = true;
            const int size = static_cast<int>(character.size());
            if (m_alignRight) {
                keepCaretVisible();
            } else {
                setColumn(column + size <= visibleChars() ? column + size : scrollRightColumn(0));
            }
        }
        if (!changed) {
            return;
        }
        raise(context, "TextChanged");
    }

    void TextBox::onMouseDown(UiContext& context, MouseButton button, core::Point local) {
        (void)button;
        (void)local;
        m_caretBlinkMs = context.nowMs;
        m_caretVisible = true;
    }

    void TextBox::onMouseUp(UiContext& context, MouseButton button, core::Point local) {
        Panel::onMouseUp(context, button, local);
        if (button == MouseButton::Left) {
            raise(context, "Click");
        }
    }

    void TextBox::onTextInput(UiContext& context, const std::string& text) {
        if (!m_readOnly) {
            insert(context, text);
        }
    }

    void TextBox::onKeyDown(UiContext& context, const KeyEvent& key) {
        if (m_readOnly) {
            return;
        }
        switch (key.key) {
            case Key::Backspace: // delete the character before the caret
                if (m_caret > 0) {
                    const int column = static_cast<int>(m_caret - std::min(m_scroll, m_caret));
                    m_text.erase(--m_caret, 1);
                    if (m_alignRight) {
                        keepCaretVisible();
                    } else {
                        setColumn(column > 0 ? column - 1 : scrollLeftColumn());
                    }
                    raise(context, "TextChanged");
                }
                break;
            case Key::Delete: // delete the character at the caret
                if (m_caret < m_text.size()) {
                    const int column = static_cast<int>(m_caret - std::min(m_scroll, m_caret));
                    m_text.erase(m_caret, 1);
                    if (!m_alignRight && column + 1 > visibleChars()) {
                        setColumn(scrollRightColumn(0));
                    }
                    raise(context, "TextChanged");
                }
                break;
            case Key::Left: // caret one character left
                if (m_caret > 0) {
                    const int column = static_cast<int>(m_caret - std::min(m_scroll, m_caret));
                    --m_caret;
                    if (m_alignRight) {
                        keepCaretVisible();
                    } else {
                        setColumn(column > 0 ? column - 1 : scrollLeftColumn());
                    }
                }
                break;
            case Key::Right: // caret one character right
                if (m_caret < m_text.size()) {
                    const int column = static_cast<int>(m_caret - std::min(m_scroll, m_caret));
                    ++m_caret;
                    if (m_alignRight) {
                        keepCaretVisible();
                    } else {
                        setColumn(column + 1 > visibleChars() ? scrollRightColumn(1) : column + 1);
                    }
                }
                break;
            case Key::Home: // caret to the start
                m_caret = 0;
                m_scroll = 0;
                break;
            case Key::End: // caret to the end
                m_caret = m_text.size();
                if (m_alignRight) {
                    keepCaretVisible();
                } else {
                    setColumn(visibleChars() > static_cast<int>(m_text.size()) ? static_cast<int>(m_text.size()) : visibleChars());
                }
                break;
            case Key::Enter: // raise the Enter event
                raise(context, "Enter");
                break;
            case Key::Up: // raise the Up event
                raise(context, "Up");
                break;
            case Key::Down: // raise the Down event
                raise(context, "Down");
                break;
            case Key::V: // Ctrl+V pastes when AllowPaste is set
                if (key.ctrl && m_allowPaste && context.clipboardText) {
                    insert(context, context.clipboardText());
                }
                break;
            default: // other keys are ignored here
                break;
        }
    }

    void TextBox::onMouseWheel(UiContext& context, int steps) {
        if (steps != 0) {
            raise(context, steps > 0 ? "WheelUp" : "WheelDown");
        }
    }

    void TextBox::onKeyUp(UiContext& context, const KeyEvent& key) {
        if (!m_readOnly && key.key == Key::Tab) {
            raise(context, "Tab");
        }
    }

    void TextBox::draw(DrawContext& context, core::Point screenPosition) {
        Panel::draw(context, screenPosition);
        const int textY = screenPosition.y + (bounds().height - gfx::TextRenderer::kLineHeight) / 2;
        const std::string shown = m_password ? std::string(m_text.size(), '*') : m_text;
        const std::string cut = shown.substr(std::min(m_scroll, shown.size()), static_cast<std::size_t>(visibleChars()));
        const std::string visible = m_shown ? m_shown(cut) : cut;
        // 2: padding in pixels at the right edge
        const int rightPad = (bounds().width - kTextLeft - 2) % kCharWidth + 2;
        const int textX =
            m_alignRight ? screenPosition.x + bounds().width - rightPad - static_cast<int>(visible.size()) * kCharWidth : screenPosition.x + kTextLeft;
        if (!visible.empty()) {
            context.canvas.drawText({textX, textY}, visible, {m_fontColor, gfx::TextEffect::Plain, std::nullopt}, bounds().width);
        }

        if (m_readOnly || context.focusedId != id()) {
            return;
        }
        if (context.nowMs > m_caretBlinkMs + kBlinkMs) {
            m_caretBlinkMs = context.nowMs;
            m_caretVisible = !m_caretVisible;
        }
        if (m_caretVisible) {
            const int column = static_cast<int>(m_caret - m_scroll);
            context.canvas.drawPicture(
                m_caretPicture,
                {textX + column * kCharWidth - 1, textY},
                core::Rect{0, 0, 7, gfx::TextRenderer::kLineHeight}
            ); // caret picture is 7 pixels wide
        }
    }

}
