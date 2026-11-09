#ifndef NORTH_ISLAND_UI_TEXT_BOX_H
#define NORTH_ISLAND_UI_TEXT_BOX_H

#include "ui/Panel.h"

#include <cstdint>
#include <functional>
#include <string>

namespace ui {

    /// @brief A one-line text field on a tiled skin with a blinking caret, scrolling like the original TSe_Editor.
    class TextBox : public Panel {
        public:
            /// @brief Which characters the field accepts.
            enum class CharFilter { Any, Digits, Alphanumeric };

            /// @brief Creates the text box and declares its events.
            TextBox();

            /// @brief Reads a layout attribute (FontColor, MaxLength, Password, Align, ReadOnly, AllowPaste, Caret, CharFilter).
            /// @param name Attribute name.
            /// @param value Attribute text.
            /// @param context Load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves the caret picture after loading.
            /// @param context Load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Restarts the caret blink.
            /// @param context UI context.
            /// @param button Button pressed.
            /// @param local Mouse in the box's coordinates.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Raises Click when the left button is let go.
            /// @param context UI context.
            /// @param button Button released.
            /// @param local Mouse in the box's coordinates.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Handles editing keys (Backspace, Delete, arrows, Home, End, Enter, Ctrl+V).
            /// @param context UI context.
            /// @param key The key event.
            void onKeyDown(UiContext& context, const KeyEvent& key) override;

            /// @brief Raises Tab when the Tab key is released.
            /// @param context UI context.
            /// @param key The key event.
            void onKeyUp(UiContext& context, const KeyEvent& key) override;

            /// @brief Inserts typed text unless the box is read-only.
            /// @param context UI context.
            /// @param text The typed text.
            void onTextInput(UiContext& context, const std::string& text) override;

            /// @brief Raises WheelUp or WheelDown.
            /// @param context UI context.
            /// @param steps Wheel steps (positive: up).
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Tells whether the box takes typed text.
            /// @return True unless read-only.
            bool takesText() const override { return !m_readOnly; }

            /// @brief Gives the text.
            /// @return The text.
            const std::string& text() const { return m_text; }

            /// @brief Replaces the text and puts the caret at its end.
            /// @param text The new text.
            void setText(const std::string& text);

            /// @brief Replaces the text with the caret at the start, so the beginning of a long text shows.
            /// @details Original: a read-only field the game fills with SetEditorText and no typing.
            /// @param text The new text.
            void setTextFromStart(const std::string& text);

            /// @brief Empties the text.
            void clear() { setText({}); }

            /// @brief Sets whether the box is read-only.
            /// @param readOnly True for read-only.
            void setReadOnly(bool readOnly) { m_readOnly = readOnly; }

            /// @brief Tells whether the box is read-only.
            /// @return True when it is.
            bool isReadOnly() const { return m_readOnly; }

            /// @brief Sets the text colour.
            /// @param color The colour.
            void setFontColor(core::Color color) { m_fontColor = color; }

            /// @brief Sets how the characters shown are drawn (the chat line blanks its emote codes under their icons); by default as they are.
            /// @param shown Maps the visible slice of the text to what is drawn.
            void setShownText(std::function<std::string(const std::string&)> shown) { m_shown = std::move(shown); }

            /// @brief Gives the first character shown (FCaretPos - FCaretCol).
            /// @return Index of the first shown character.
            std::size_t firstShown() const { return m_scroll; }

            /// @brief Tells how many characters fit in the box.
            /// @return The number of visible characters.
            int visibleChars() const;

        protected:
            /// @brief Draws the skin, the text and the caret.
            /// @param context Draw context.
            /// @param screenPosition Screen position of the box.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief Tells whether the filter and control-character rules allow a character.
            /// @param c The character.
            /// @return True when accepted.
            bool accepts(char c) const;

            /// @brief Inserts text at the caret, character by character, honouring the filter and the length limit.
            /// @param context UI context.
            /// @param text Text to insert.
            void insert(UiContext& context, const std::string& text);

            /// @brief Scrolls the text so the caret is in view (used by right-aligned boxes).
            void keepCaretVisible();

            /// @brief Gives the caret column after it runs off the right edge, as TSe_Editor scrolls (EditorScrollRightCaretCol).
            /// @param shift 1 after a Right key, else 0.
            /// @return The caret column.
            int scrollRightColumn(int shift) const;

            /// @brief Gives the caret column after it runs off the left edge (EditorScrollLeftCaretCol).
            /// @return The caret column.
            int scrollLeftColumn() const;

            /// @brief Sets the first shown character from the caret's column (FirstPos = FCaretPos - FCaretCol + 1).
            /// @param column Caret column.
            void setColumn(int column);

            std::string m_text;
            std::size_t m_caret = 0;
            std::size_t m_scroll = 0;
            core::Color m_fontColor{255, 255, 255, 255}; // white
            std::size_t m_maxLength = 0;
            bool m_password = false;
            bool m_allowPaste = false;
            bool m_readOnly = false;
            bool m_alignRight = false;
            CharFilter m_filter = CharFilter::Any;
            std::function<std::string(const std::string&)> m_shown;
            std::string m_caretName = "inputCursor";
            gfx::PictureId m_caretPicture;
            bool m_caretVisible = true;
            std::uint64_t m_caretBlinkMs = 0;
    };

}

#endif
