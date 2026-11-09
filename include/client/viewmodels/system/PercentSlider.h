#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_PERCENT_SLIDER_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_PERCENT_SLIDER_H

#include <cstdint>

namespace client {

    /// @brief Horizontal percent slider (0..100) with a draggable thumb and press-and-hold paging.
    /// @details Original: a horizontal TSe_ScrollButton as the Remote Control sets its four percents (SetScrollBarMax(101), page 1, no track picture; the form draws the rail). The thumb is 16 pixels (twice its cap) on a 70 pixel bar, so it travels 54.
    class PercentSlider {
        public:
            static constexpr int kWidth = 70; // slider width, px
            static constexpr int kThumbSize = 16; // thumb width, px
            static constexpr int kTrackLength = kWidth - kThumbSize;
            static constexpr int kMaximum = 100; // slider value at the far end, percent

            /// @brief Gets the current percent.
            /// @return Value 0..100.
            int position() const { return m_position; }

            /// @brief Sets the percent and moves the thumb to match.
            /// @details Original: SetScrollBarPos.
            /// @param position New value 0..100.
            void setPosition(int position);

            /// @brief Gets the thumb's left edge on the bar.
            /// @return Offset in pixels, 0..kTrackLength.
            int thumbPos() const { return m_thumbPos; }

            /// @brief Tells whether a point of the bar lies on the thumb.
            /// @param x Horizontal point on the bar.
            /// @return True when x is within the thumb.
            bool onThumb(int x) const { return x >= m_thumbPos && x < m_thumbPos + kThumbSize; }

            /// @brief Tells whether the thumb is being dragged.
            /// @return True while dragging.
            bool isDragging() const { return m_dragging; }

            /// @brief Gives the thumb picture state: 2 held (dragged or paging), 1 under the mouse, else 0.
            /// @details Original: RenderWithChildren.
            /// @param mouseOnThumb Whether the mouse is over the thumb.
            /// @return Thumb state 0..2.
            int thumbState(bool mouseOnThumb) const;

            /// @brief Starts a drag on the thumb, or starts paging when pressed beside it.
            /// @details Original: HandleMouseDown.
            /// @param x Horizontal point of the bar that was pressed.
            void press(int x);

            /// @brief Moves the dragged thumb and updates the percent.
            /// @details Original: HandleCapturedMouseMove with the button held.
            /// @param x Current horizontal mouse point on the bar.
            void drag(int x);

            /// @brief Ends any drag or paging.
            /// @details Original: HandleMouseUp.
            void release();

            /// @brief Repeats the paging step every 100 ms while the mouse stays on the bar but off the thumb.
            /// @details Original: RenderWithChildren.
            /// @param nowMs Current time in milliseconds.
            /// @param mouseOnBar Whether the mouse is over the bar.
            /// @param mouseOnThumb Whether the mouse is over the thumb.
            void update(std::uint64_t nowMs, bool mouseOnBar, bool mouseOnThumb);

        private:
            /// @brief Recomputes the thumb position from the percent.
            void placeThumb();

            int m_position = 0;
            int m_thumbPos = 0;
            bool m_dragging = false;
            int m_dragStartX = 0;
            int m_dragStartThumb = 0;
            bool m_pagingUp = false;
            bool m_pagingDown = false;
            std::uint64_t m_lastRepeatMs = 0;
    };

}

#endif
