#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_SCROLLED_ROWS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_SCROLLED_ROWS_H

#include "ui/DrawingArea.h"
#include "ui/ScrollBar.h"
#include "ui/Screen.h"

#include <string>

namespace client {

    /// @brief A tab's rows, taller than the box that clips them, moved by a scroll bar and the mouse wheel.
    class ScrolledRows {
        public:
            /// @brief Finds the rows' elements in a loaded layout.
            /// @param screen Screen the layout is loaded into.
            /// @param rows Name of the panel holding the rows.
            /// @param wheel Name of the area under the rows that takes the mouse wheel.
            /// @param bar Name of the scroll bar.
            ScrolledRows(
                ui::Screen& screen,
                const std::string& rows,
                const std::string& wheel,
                const std::string& bar
            );

            /// @brief Sets how tall the rows are; the bar is hidden while they all fit.
            /// @param height The height of the rows in pixels.
            void setContentHeight(int height);

            /// @brief Moves the rows.
            /// @param position The new scroll position in steps, clamped to the range.
            void scrollTo(int position);

            /// @brief Moves the rows by the mouse wheel.
            /// @param notches The wheel notches (positive: up).
            void wheel(int notches);

        private:
            ui::Element& m_rows;
            ui::Element& m_wheel;
            ui::ScrollBar& m_bar;
            int m_steps = 0;
    };

}

#endif
