#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_TOP100_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_TOP100_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/events/Top100.h"
#include "core/Point.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>
#include <vector>

namespace client {

    /// @brief The Top 100 window with tabs for the level, money, Q&A, PK and event rankings.
    /// @details Original: Top100Window.xml, TLD_Top100Form; the server opens it with 84/1.
    class Top100Window {
        public:
            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param ranks The ranking lists model.
            Top100Window(
                GameServices& services,
                ui::Screen& screen,
                Top100& ranks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window and opens the ranks.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window.
            /// @details Original: HideTop100Form.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief A place the page shows: its row on the window and its place in the list.
            struct Row {
                int row;
                int place;
            };

            /// @brief Sets which controls show and what the labels say, and draws the event lists' picture.
            /// @param canvas Target canvas.
            /// @param at Top left of the layout area on the canvas.
            void layout(gfx::Canvas& canvas, core::Point at);

            /// @brief Lists the places of the page that have their data.
            /// @return The rows to show.
            std::vector<Row> shownRows() const;

            /// @brief Paints the page numbers and the list of the shown kind.
            /// @details Original: DrawControl, DrawLevelPage and the others.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the level ranking.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            /// @param rows Places to paint.
            void paintLevels(gfx::Canvas& canvas, core::Point at, const std::vector<Row>& rows);

            /// @brief Paints the money, Q&A and PK rankings.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            /// @param rows Places to paint.
            void paintNames(gfx::Canvas& canvas, core::Point at, const std::vector<Row>& rows);

            /// @brief Paints the event rankings.
            /// @param canvas Target canvas.
            /// @param at Top left of the page area on the canvas.
            /// @param rows Places to paint.
            void paintEvents(gfx::Canvas& canvas, core::Point at, const std::vector<Row>& rows);

            GameServices& m_services;
            ui::Screen& m_screen;
            Top100& m_ranks;
            ui::Window& m_window;
            const ui::Spot& m_eventBackgroundSpot;
            const ui::Spot& m_pageNumbersSpot;
            const ui::Spot& m_rowsSpot;
            const ui::Spot& m_myRankSpot;
            const ui::Spot& m_levelWordSpot;
            const ui::Spot& m_levelNameSpot;
            const ui::Spot& m_moneyIconSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_eventNameSpot;
            const ui::Spot& m_textColorSpot;
            std::vector<std::reference_wrapper<ui::TextBox>> m_numbers;
            std::vector<std::reference_wrapper<ui::DrawingArea>> m_jobs;
    };

}

#endif
