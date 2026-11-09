#include "client/rendering/world/ChatBubble.h"

#include "client/data/EmoteCodes.h"
#include "client/rules/TextFilter.h"
#include "gfx/TextStyle.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::uint64_t kShownMs = 5000; // how long a bubble stays up, ms
        constexpr std::uint64_t kIconFrameMs = 210; // time per frame of an emote icon, ms
        constexpr int kCharWidth = 8; // width of one character cell, px
        constexpr int kRowChars = 20; // characters per bubble row
        constexpr std::size_t kMostChars = 60; // most characters shown in a bubble
        constexpr int kRowPitch = 23; // vertical distance between bubble rows, px
        constexpr int kIconSize = 16; // emote icon frame size, px (16 x 16)
        const core::Color kTextColor = core::Color::fromRgb565(0x841); // RGB565 0x841: near-black dark text

    }

    void ChatBubble::show(const std::string& text, std::uint64_t nowMs, bool speakerFights) {
        m_text.clear();
        if (!speakerFights && !isTextClean(text)) {
            return;
        }
        m_text = text;
        m_sinceMs = nowMs;
    }

    void ChatBubble::draw(
        gfx::Canvas& canvas,
        gfx::PictureLibrary& pictures,
        core::Point feet,
        core::Size body,
        std::uint64_t nowMs
    ) {
        if (m_text.empty()) {
            return;
        }
        if (nowMs >= m_sinceMs + kShownMs) {
            m_text.clear();
            return;
        }
        const auto balloon = pictures.find("TalkForm1");
        const int length = static_cast<int>(m_text.size()) * kCharWidth;
        // 160: widest text line in pixels; 15 and 55: x offsets of the text and of a narrow balloon from the body; 58: text height above the feet; 10: balloon top above the text
        const int textWidth = std::min(length, 160);
        const int textX = feet.x - body.width / 2 + 15 - textWidth / 2;
        const int textY = feet.y - body.height - 58;
        int x = feet.x - body.width / 2 + 15 - 55;
        const int y = textY - 10;
        int width = 0;
        int height = 0;
        int rows = 1;
        if (length > 80 && length <= 160) { // text between 80 and 160 pixels widens the balloon
            x = textX - 15;
            width = length - 80;
        }
        if (length > 160) { // more than 160 pixels wraps into rows
            width = 80; // widest extra balloon width in pixels
            x = textX - 15;
            rows = (static_cast<int>(m_text.size()) + kRowChars - 1) / kRowChars;
            height = (rows - 1) * kRowPitch;
        }
        if (balloon) {
            const auto piece = [&](core::Rect source, core::Point at) {
                if (source.width > 0 && source.height > 0) {
                    canvas.drawPicture(*balloon, at, source);
                }
            };
            // the balloon pieces are cut from TalkForm1 (edges in pixels: 95 = narrowest balloon width, 309..324 = right edge, 10 = top edge height, 26 = body height, 158..168 = bottom edge rows, 188..233 x 168..185 = tail)
            piece(core::Rect::fromEdges(0, 0, width + 95, 10), {x, y - height});
            piece(core::Rect::fromEdges(309, 0, 324, 10), {x + 95 + width, y - height});
            piece(core::Rect::fromEdges(309, 158, 324, 168), {x + 95 + width, y + 26});
            piece(core::Rect::fromEdges(0, 10, 15, height + 26), {x, y - height + 10});
            piece(core::Rect::fromEdges(15, 10, width + 95, height + 26), {x + 15, y - height + 10});
            piece(core::Rect::fromEdges(309, 10, 324, height + 26), {x + 95 + width, y - height + 10});
            const int bottom = std::min(width + 95, 188); // width in pixels of the bottom edge piece
            piece(core::Rect::fromEdges(0, 158, bottom, 168), {x, y + 26});
            if (width + 95 > 188) { // a balloon wider than the bottom edge piece repeats the edge
                const int rest = width + 95 - 188;
                piece(core::Rect::fromEdges(1, 158, rest + 1, 168), {x + 95 + width + rest, y + 26});
            }
            piece(core::Rect::fromEdges(188, 168, 233, 185), {feet.x - body.width / 2 + 10, y + 33});
        }
        const int iconFrame = static_cast<int>(nowMs / kIconFrameMs % 2); // the emote icon has two frames
        const core::Rect iconSource{0, iconFrame * kIconSize, kIconSize, kIconSize};
        const auto drawIcon = [&](int number, int column, int rowY) {
            if (const auto icon = pictures.find("icon_expre_" + std::to_string(number))) {
                canvas.drawPicture(*icon, {textX + (column - 1) * kCharWidth, rowY}, iconSource);
            }
        };
        std::size_t next = 0;
        for (int row = rows; row >= 1; --row) {
            std::string line;
            int column = 1;
            const int rowY = textY - (row - 1) * kRowPitch;
            while (next < m_text.size() && next < kMostChars && column <= kRowChars) {
                bool emote = false;
                bool full = false;
                for (int number = 1; number <= kEmoteCount; ++number) {
                    if (next + 1 >= m_text.size()) {
                        break;
                    }
                    const std::string& code = emoteCode(number);
                    if (m_text[next] != code[0] || m_text[next + 1] != code[1]) {
                        continue;
                    }
                    if (column % kRowChars == 0) {
                        line += "  ";
                        drawIcon(number, column, rowY);
                        emote = true;
                        next += 2;
                        column += 2;
                        break;
                    }
                    if (column % (kRowChars + 1) == 0) {
                        full = true;
                        break;
                    }
                    drawIcon(number, column, rowY);
                    line += "  ";
                    emote = true;
                    next += 2;
                    column += 2;
                }
                if (full) {
                    break;
                }
                if (!emote) {
                    line += m_text[next];
                    ++next;
                    ++column;
                }
            }
            canvas.drawText(
                {textX, rowY},
                line,
                {kTextColor, gfx::TextEffect::Plain, std::nullopt},
                static_cast<int>(line.size()) * kCharWidth,
                12
            ); // text box height in pixels
        }
    }

}
