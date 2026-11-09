#ifndef NORTH_ISLAND_UI_MEMO_H
#define NORTH_ISLAND_UI_MEMO_H

#include "ui/Element.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ui {

    /// @brief A multi-line text field (TSe_Memo) with a caret, wrapping, scrolling, emote pictures and an optional read-only mode.
    /// @details Original: TSe_Memo (SetMemoText, ClearMemoText, MemoInputChar, AppendMemoLine, IsMemoFull, FLineChars, FVisibleRows, TSe_Memo.DrawControl).
    class Memo : public Element {
        public:
            static constexpr int kEmotes = 31; // number of emote pictures (icon_expre_1 .. icon_expre_31)

            /// @brief Creates a memo with the hand cursor and the Typed event.
            Memo();

            /// @brief Reads the Memo attributes (FontColor, Effect, MaxChars, ReadOnly, Caret, JoinRows).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Looks up the caret and emote pictures.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Whether the point lies in the memo's rectangle.
            /// @param local The point relative to the memo.
            /// @param pictures The picture library (unused).
            /// @return True when inside.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Restarts the caret blink on a press (the caret does not follow the mouse).
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the memo.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Scrolls the rows by the wheel steps.
            /// @param context The UI context.
            /// @param steps The wheel steps (positive up).
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Moves the caret and edits the text (arrows, Home, End, Insert, Backspace, Delete, Enter).
            /// @details Original: HandleKeyDown (Ctrl+V does nothing: the game's PasteMemoText is empty).
            /// @param context The UI context.
            /// @param key The key event.
            void onKeyDown(UiContext& context, const KeyEvent& key) override;

            /// @brief Types the text and raises Typed.
            /// @param context The UI context.
            /// @param text The typed text.
            void onTextInput(UiContext& context, const std::string& text) override;

            /// @brief Whether the memo takes typed text.
            /// @return True unless read-only.
            bool takesText() const override { return !m_readOnly; }

            /// @brief The whole text: the rows ended with Enter separated by carriage returns.
            /// @details Original: FFullText.
            /// @return The text.
            const std::string& text() const { return m_text; }

            /// @brief Replaces the text and puts the caret back at the start.
            /// @details Original: SetMemoText (mode 0).
            /// @param text The new text.
            void setText(const std::string& text);

            /// @brief Empties the text.
            /// @details Original: ClearMemoText.
            void clear() { setText({}); }

            /// @brief Types each character as if typed (MemoInputChar); nothing when read-only.
            /// @param text The characters to type.
            void type(const std::string& text);

            /// @brief Appends a row ended with a carriage return and puts the caret back at the start.
            /// @details Original: AppendMemoLine.
            /// @param line The row text.
            void appendLine(const std::string& line) { setText(m_text + line + '\r'); }

            /// @brief Scrolls a page of rows on (the paging of a scroll bar the memo is tied to).
            /// @details Original: ScrollBarPageDown.
            void pageDown();

            /// @brief Scrolls a page of rows back.
            /// @details Original: ScrollBarPageUp.
            void pageUp();

            /// @brief Sets the text colour.
            /// @param color The colour.
            void setFontColor(core::Color color) { m_fontColor = color; }

            /// @brief Makes the memo read-only (set by a form that shows a saved text).
            /// @details Original: FReadOnly.
            /// @param readOnly True for read-only.
            void setReadOnly(bool readOnly) { m_readOnly = readOnly; }

            /// @brief The row the caret is in.
            /// @details Original: FCurLine.
            /// @return The row text.
            const std::string& currentLine() const { return caretLine(); }

        protected:
            /// @brief Draws the visible rows (or the joined text) and the blinking caret.
            /// @param context The draw context.
            /// @param screenPosition The memo's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief Cuts the text into rows.
            /// @details Original: SetMemoText.
            /// @param keepCaret True to leave the caret where it is (mode 2).
            void layout(bool keepCaret);

            /// @brief Where the caret is in the text.
            /// @return The byte offset.
            std::size_t caretOffset() const;

            /// @brief The caret's row of the text.
            /// @return The row text.
            const std::string& caretLine() const;

            /// @brief Whether the rows hold MaxChars characters already.
            /// @details Original: IsMemoFull.
            /// @return True when full.
            bool isFull() const;

            /// @brief How many characters a row holds.
            /// @details Original: FLineChars.
            /// @return The characters per row.
            int lineChars() const;

            /// @brief How many rows are visible.
            /// @details Original: FVisibleRows.
            /// @return The row count.
            int visibleRows() const;

            /// @brief Draws the emote pictures of a row and blanks their codes in the text.
            /// @param context The draw context.
            /// @param row The row text.
            /// @param at The row's top left on the screen.
            /// @return The row text to draw (emote codes blanked).
            std::string drawRow(DrawContext& context, const std::string& row, core::Point at) const;

            /// @brief Types one character at the caret (MemoInputChar).
            /// @details Original: MemoInputChar.
            /// @param c The character; non-printable ones are ignored.
            void insert(char c);

            /// @brief Scrolls one row down (MemoScrollDown).
            /// @details Original: MemoScrollDown.
            void scrollDown();

            /// @brief Scrolls one row up (MemoScrollUp).
            /// @details Original: MemoScrollUp.
            void scrollUp();

            std::string m_text;
            std::vector<std::string> m_lines{std::string()};
            int m_caretRow = 0;
            int m_caretCol = 0;
            int m_topLine = 0;
            bool m_insertMode = true;
            core::Color m_fontColor = core::Color::fromRgb565(2113); // RGB565 0x0841: near black (8, 8, 8)
            bool m_shadow = true;
            std::size_t m_maxChars = 0;
            bool m_readOnly = false;
            bool m_joinRows = false;
            std::string m_caretName = "inputCursor2";
            gfx::PictureId m_caretPicture;
            std::array<gfx::PictureId, kEmotes + 1> m_emotePictures{};
            std::function<int(const std::string&, std::size_t)> m_emoteAt;
            bool m_caretVisible = false;
            std::uint64_t m_caretBlinkMs = 0;
    };

}

#endif
