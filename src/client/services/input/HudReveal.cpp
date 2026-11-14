#include "client/services/input/HudReveal.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace client {

    namespace {

        constexpr double kSlideMs = 200.0; // how long a piece takes to slide off or back
        constexpr double kLongestFrameMs = 100.0; // a longer gap between frames moves the slide no further than this
        constexpr int kMostMarks = 3; // the most elements that tell where a piece lies

        /// @brief Which edge a piece leaves by.
        enum class Edge { Left, Right, Top, Bottom, Nearest };

        /// @brief A piece of the HUD: its layout, the elements that tell where it lies, and how it leaves.
        struct Piece {
            const char* layout;
            const char* marks[kMostMarks]; // those shown are taken together; null ends the list
            Edge edge;
            int extra; // further to go than the marks reach, for what sticks out past them, px
            HudReveal::Group group;
        };

        constexpr Piece kPieces[] = {
            {"MainButtons", {"MainButtonsBackground", "MainItem", "MainSocial"}, Edge::Bottom, 30, HudReveal::Group::Bars},
            {"FunctionButtons", {"FunctionBackground", nullptr, nullptr}, Edge::Top, 10, HudReveal::Group::Bars},
            {"HotKeyBar", {"HotKeyBackground", "HotKeyMinimize", nullptr}, Edge::Nearest, 10, HudReveal::Group::Shortcuts},
            {"MiniMap", {"MiniMapPicture", "MiniMapMinimize", nullptr}, Edge::Nearest, 10, HudReveal::Group::Bars},
            {"TileMiniMap", {"TileMiniMapPicture", "TileMiniMapMinimize", nullptr}, Edge::Nearest, 10, HudReveal::Group::Bars},
            {"MainStatus", {"StatusHints", "StatusVehicle", nullptr}, Edge::Left, 10, HudReveal::Group::Status},
            // the effect icons stand above the bottom right buttons and have no frame of their own to measure
            {"TimeEffectLayer", {"MainButtonsBackground", nullptr, nullptr}, Edge::Bottom, 170, HudReveal::Group::Status},
            {"ChatWindow", {"ChatForm", nullptr, nullptr}, Edge::Bottom, 20, HudReveal::Group::Chat},
            {"InputBar", {"InputBackground", nullptr, nullptr}, Edge::Bottom, 10, HudReveal::Group::Chat},
        };

        /// @brief Gives how far a piece must go to be off the screen.
        /// @param screen The screen.
        /// @param piece The piece.
        /// @return The whole slide in screen pixels, nothing while none of its marks is shown.
        std::optional<core::Point> wholeSlide(ui::Screen& screen, const Piece& piece) {
            std::optional<core::Rect> lies;
            for (const char* mark : piece.marks) {
                const auto rect = mark != nullptr ? screen.screenRectOf(mark) : std::nullopt;
                if (!rect) {
                    continue;
                }
                if (!lies) {
                    lies = rect;
                    continue;
                }
                const int right = std::max(lies->x + lies->width, rect->x + rect->width);
                const int bottom = std::max(lies->y + lies->height, rect->y + rect->height);
                lies->x = std::min(lies->x, rect->x);
                lies->y = std::min(lies->y, rect->y);
                lies->width = right - lies->x;
                lies->height = bottom - lies->y;
            }
            if (!lies) {
                return std::nullopt;
            }
            const core::Rect& view = screen.view();
            const int extra = static_cast<int>(std::ceil(static_cast<float>(piece.extra) * screen.hudScale()));
            const int toLeft = lies->x + lies->width - view.x + extra;
            const int toRight = view.x + view.width - lies->x + extra;
            const int toTop = lies->y + lies->height - view.y + extra;
            const int toBottom = view.y + view.height - lies->y + extra;
            Edge edge = piece.edge;
            if (edge == Edge::Nearest) {
                const int least = std::min(std::min(toLeft, toRight), std::min(toTop, toBottom));
                edge = least == toRight ? Edge::Right : least == toLeft ? Edge::Left : least == toTop ? Edge::Top : Edge::Bottom;
            }
            switch (edge) {
                case Edge::Left: return core::Point{-toLeft, 0}; // out by the left edge
                case Edge::Right: return core::Point{toRight, 0}; // out by the right edge
                case Edge::Top: return core::Point{0, -toTop}; // out by the top edge
                default: return core::Point{0, toBottom}; // out by the bottom edge
            }
        }

    }

    void HudReveal::update(double nowMs, ui::Screen& screen, bool hidden, bool hideStatus, bool hideChat, bool showShortcuts) {
        const float step = static_cast<float>(std::clamp(nowMs - m_lastMs, 0.0, kLongestFrameMs) / kSlideMs);
        m_lastMs = nowMs;
        const bool goes[static_cast<int>(Group::Count)] = {hidden, hidden && !showShortcuts, hidden && hideStatus, hidden && hideChat};
        for (int group = 0; group < static_cast<int>(Group::Count); ++group) {
            m_away[group] = goes[group] ? std::min(1.0f, m_away[group] + step) : std::max(0.0f, m_away[group] - step);
        }
        for (const Piece& piece : kPieces) {
            const float away = m_away[static_cast<int>(piece.group)];
            if (away <= 0.0f) {
                screen.slideLayout(piece.layout, {}, false);
                continue;
            }
            if (away >= 1.0f) {
                screen.slideLayout(piece.layout, {}, true);
                continue;
            }
            const auto whole = wholeSlide(screen, piece);
            if (!whole) {
                screen.slideLayout(piece.layout, {}, goes[static_cast<int>(piece.group)]); // nothing to measure: it goes or comes at once
                continue;
            }
            const float eased = away * away * (3.0f - 2.0f * away); // slow at both ends
            const core::Point slide{static_cast<int>(std::lround(whole->x * eased)), static_cast<int>(std::lround(whole->y * eased))};
            // a slide that rounds to nothing would let the mouse back on the piece a frame early; that is fine
            screen.slideLayout(piece.layout, slide, false);
        }
    }

}
