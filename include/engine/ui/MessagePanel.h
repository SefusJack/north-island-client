#ifndef NORTH_ISLAND_UI_MESSAGE_PANEL_H
#define NORTH_ISLAND_UI_MESSAGE_PANEL_H

#include "ui/Element.h"
#include "ui/MessageTags.h"
#include "ui/TiledFrame.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ui {

    /// @brief A popup for short messages on a tiled skin sized to its text; red tagged parts, timed lines, closes on click.
    /// @details Original: TRe_TalkMsgFormPlus_1 (queued) and TRe_TalkMsgFormPlus_2 (unqueued); size from TSe_TalkMsgFormPlus.CalcMsgPanelSize.
    class MessagePanel : public Element {
        public:
            /// @brief Reads the MessagePanel attributes (Skin, SkinSource, Borders, Margins, WrapColumns, MaxLines, LineHeight, LineSpacing, FontColor, Queued).
            /// @param name The attribute name.
            /// @param value The attribute text.
            /// @param context The load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Builds the tiled skin.
            /// @param context The load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Whether the point lies in the visible panel.
            /// @param local The point relative to the element.
            /// @param pictures The picture library (unused).
            /// @return True when the panel takes the mouse there.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Drops expired lines and closes the panel when the newest message expires.
            /// @param context The UI context.
            void update(UiContext& context) override;

            /// @brief Closes the panel when it is clicked.
            /// @param context The UI context.
            /// @param button The button.
            /// @param local The point relative to the element.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Adds a message (text in the game's encoding; '\r' or '\n' start new lines).
            /// @param text The message text, with tags.
            /// @param durationMs How long the message stays.
            /// @param nowMs The current time in ms.
            void show(const std::string& text, int durationMs, std::uint64_t nowMs);

            /// @brief Adds the text's lines below those shown (the qol party's grouped messages), up to mostLines.
            /// @param text The message text, with tags.
            /// @param durationMs How long the message stays.
            /// @param nowMs The current time in ms.
            /// @param mostLines The most lines kept at once (at least MaxLines).
            void append(const std::string& text, int durationMs, std::uint64_t nowMs, std::size_t mostLines);

            /// @brief Clears the lines and hides the panel.
            void close();

            /// @brief Puts a shown panel whose top lies above minTop down to it, and keeps that place.
            /// @details Original: TSe_ShortMsgPanel.RenderWithChildren moves FTargetTop; only panels that set it again when they show (the main talk box and the HL ones) lose it.
            /// @param minTop The least top position.
            void keepBelow(int minTop);

        protected:
            /// @brief Draws the skin and the lines centred, red pieces in red.
            /// @param context The draw context.
            /// @param screenPosition The element's top left on the screen.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief One displayed line with its expiry time and colour pieces.
            struct Line {
                std::string text;
                std::uint64_t expiresMs;
                std::vector<MessageTags::Segment> pieces;
            };

            /// @brief The panel's rectangle: sized to its longest line and line count, centred in the element's width.
            /// @param screenPosition The element's top left on the screen.
            /// @return The panel rectangle on the screen.
            core::Rect panelArea(core::Point screenPosition) const;

            TiledFrame m_skin;
            std::string m_skinName;
            std::optional<core::Rect> m_skinSource;
            std::array<int, 4> m_borders = {50, 32, 60, 32}; // skin frame borders, px: left, top, right, bottom
            std::array<int, 4> m_margins = {50, 31, 50, 31}; // text margins inside the panel, px: left, top, right, bottom
            std::size_t m_wrapColumns = 32; // characters per line before wrapping
            std::size_t m_maxLines = 5; // lines kept; older ones drop off
            int m_lineHeight = 15; // line height, px
            int m_lineSpacing = 3; // extra gap between lines, px
            core::Color m_fontColor = core::Color::fromRgb565(0xFFE0); // RGB565 0xFFE0: yellow

            bool m_queued = true;

            std::vector<Line> m_lines;
            std::uint64_t m_closeAtMs = 0;
    };

}

#endif
