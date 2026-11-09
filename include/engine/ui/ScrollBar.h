#ifndef NORTH_ISLAND_UI_SCROLL_BAR_H
#define NORTH_ISLAND_UI_SCROLL_BAR_H

#include "ui/Element.h"

#include <cstdint>

namespace ui {

    /// @brief A vertical scroll bar with a track, a three-state thumb and paging on the track.
    class ScrollBar : public Element {
        public:
            /// @brief Creates the scroll bar and declares its Scroll event.
            ScrollBar();

            /// @brief Reads a layout attribute (Track, Thumb, ThumbSource, ThumbCap, TrackCap, ThumbX, ThumbInset, MinThumb).
            /// @param name Attribute name.
            /// @param value Attribute text.
            /// @param context Load context.
            void applyAttribute(const std::string& name, const std::string& value, const LoadContext& context) override;

            /// @brief Resolves the pictures and the thumb defaults after loading.
            /// @param context Load context.
            void finishLoading(const LoadContext& context) override;

            /// @brief Tells whether a point is inside the bar.
            /// @param local Point in the bar's coordinates.
            /// @param pictures Unused.
            /// @return True when inside.
            bool hitTest(core::Point local, const gfx::PictureLibrary& pictures) const override;

            /// @brief Repeats paging while the mouse is held on the track.
            /// @param context UI context.
            void update(UiContext& context) override;

            /// @brief Starts a thumb drag or track paging.
            /// @param context UI context.
            /// @param button Button pressed.
            /// @param local Mouse in the bar's coordinates.
            void onMouseDown(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Stops dragging and paging.
            /// @param context UI context.
            /// @param button Button released.
            /// @param local Mouse in the bar's coordinates.
            void onMouseUp(UiContext& context, MouseButton button, core::Point local) override;

            /// @brief Drags the thumb and raises Scroll when the position changes.
            /// @param context UI context.
            /// @param local Mouse in the bar's coordinates.
            void onMouseDrag(UiContext& context, core::Point local) override;

            /// @brief Stops dragging and paging when the mouse is released elsewhere.
            /// @param context UI context.
            void onCaptureReleased(UiContext& context) override;

            /// @brief Scrolls by lines and raises Scroll when the position changes.
            /// @param context UI context.
            /// @param steps Wheel steps (positive: up).
            void onMouseWheel(UiContext& context, int steps) override;

            /// @brief Sets how many items there are and how many are visible at once.
            /// @param count Number of items.
            /// @param page Number of items visible at once.
            void setRange(int count, int page);

            /// @brief Sets the scroll position, clamped to the range.
            /// @param position First visible item.
            void setPosition(int position);

            /// @brief Gives the scroll position.
            /// @return First visible item.
            int position() const { return m_position; }

            /// @brief Tells whether the thumb is held by the mouse (FThumbDragging).
            /// @return True while dragging.
            bool isDragging() const { return m_dragging; }

        protected:
            /// @brief Draws the track and the thumb.
            /// @param context Draw context.
            /// @param screenPosition Screen position of the bar.
            void draw(DrawContext& context, core::Point screenPosition) override;

        private:
            /// @brief Recomputes the thumb position from the scroll position.
            void placeThumb();

            /// @brief Scrolls one item up.
            void lineUp();

            /// @brief Scrolls one item down.
            void lineDown();

            /// @brief Scrolls one page up.
            void pageUp();

            /// @brief Scrolls one page down.
            void pageDown();

            /// @brief Ends dragging and paging.
            void stopTracking();

            /// @brief Tells whether a point is on the thumb.
            /// @param local Point in the bar's coordinates.
            /// @return True when on the thumb.
            bool isOnThumb(core::Point local) const { return local.y >= m_thumbPos && local.y < m_thumbPos + m_thumbSize; }

            std::string m_trackName;
            std::string m_thumbName;
            gfx::PictureId m_track;
            gfx::PictureId m_thumb;
            std::optional<core::Rect> m_thumbSource;
            int m_thumbCap = 0;
            int m_trackCap = 0;
            int m_thumbInset = 0;
            int m_minThumb = 0; // shortest thumb in pixels; the two caps when 0
            std::optional<int> m_thumbX;

            int m_maximum = 0;
            int m_page = 0;
            int m_position = 0;
            int m_thumbPos = 0;
            int m_thumbSize = 0;
            int m_trackLength = 0;

            bool m_dragging = false;
            int m_dragStartY = 0;
            int m_dragStartThumb = 0;
            bool m_pagingUp = false;
            bool m_pagingDown = false;
            bool m_mouseOverThumb = false;
            bool m_hovered = false;
            std::uint64_t m_lastRepeatMs = 0;
    };

}

#endif
