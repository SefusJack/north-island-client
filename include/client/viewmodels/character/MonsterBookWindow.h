#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_MONSTER_BOOK_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_MONSTER_BOOK_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/MonsterBook.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief Monster book window: the monsters met, ten a page; a click on a name shows the monster with its element, level and values.
    /// @details Original: MonsterBookWindow.xml, TTH_CellectionForm.
    class MonsterBookWindow {
        public:
            /// @brief Builds the window and wires its painter and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param book The monster book data.
            MonsterBookWindow(
                GameServices& services,
                ui::Screen& screen,
                const MonsterBook& book
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window, or closes it when open.
            /// @details Original: ToggleCellectionForm.
            void toggle();

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Paints the hover bar, page number and the selected monster with its values.
            /// @details Original: DrawControl (DrawCellectionHover, DrawPreviewNpc).
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Turns the page when the target page exists.
            /// @details Original: NextCellectionPage / PrevCellectionPage.
            /// @param step 1 for next, -1 for previous.
            void turnPage(int step);

            /// @brief Selects the monster of a row of the current page.
            /// @details Original: SelectCellectionRow.
            /// @param row Row on the page, 1..10.
            void selectRow(int row);

            /// @brief Shows the names of the current page in the row editors.
            void showNames();

            GameServices& m_services;
            ui::Screen& m_screen;
            const MonsterBook& m_book;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::TextBox>, MonsterBook::kPerPage> m_rows;
            const ui::Spot& m_hoverSpot;
            const ui::Spot& m_pageSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_feetSpot;
            const ui::Spot& m_elementSpot;
            const ui::Spot& m_barHpSpot;
            const ui::Spot& m_numberSpot;
            const ui::Spot& m_strengthSpot;
            const ui::Spot& m_constitutionSpot;
            const ui::Spot& m_intelligenceSpot;
            const ui::Spot& m_wisdomSpot;
            const ui::Spot& m_agilitySpot;
            const ui::Spot& m_levelSpot;
            int m_page = 1;
            int m_selected = 0;
    };

}

#endif
