#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_ADMIN_ITEMS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_ADMIN_ITEMS_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/ScrollBar.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief The qol client's admin item browser: every item of Item.dat by category or name/id search, nine to a row; the chosen one goes into the bag through the GM command "/give <id> <count>".
    /// @details Not in the original game (AdminItemsWindow.xml, no original form). The command is sent as a chat line the chat never shows. Only a GM in admin mode has it (16/200); the ADMIN button stays up while the mode is on.
    class AdminItemsWindow {
        public:
            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param sendCommand Sends one GM command line to the server.
            AdminItemsWindow(
                GameServices& services,
                ui::Screen& screen,
                std::function<void(const std::string&)> sendCommand
            );

            /// @brief Switches admin mode: on opens the browser and shows the ADMIN button, off closes both.
            /// @details Original: server packet 16/200 sets the mode.
            /// @param on Whether admin mode is on.
            void setAdminMode(bool on);

            /// @brief Tells whether the browser is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

        private:
            /// @brief Item category filter, in the order of the AdminItemsKind buttons.
            enum class Kind { All, Weapons, Hats, Clothes, Gloves, Shoes, Jewelry, Usables, Food, Quest, Boxes, Materials, Furniture, Vehicles, Other };

            /// @brief Shows the browser (admin mode only), listing the items on first use.
            void open();

            /// @brief Hides the browser.
            void close();

            /// @brief Selects a category and refilters.
            /// @param kind Category selected.
            void showKind(Kind kind);

            /// @brief Collects the items of the category that match the search, then shows the first rows of them.
            void filter();

            /// @brief Scrolls the grid to a top row.
            /// @param row Wanted top row (clamped).
            void scrollTo(int row);

            /// @brief Refills the grid cells and the count label.
            void refresh();

            /// @brief Chooses the item of a grid cell.
            /// @param cell Cell number (1-based).
            void pick(int cell);

            /// @brief Sends the GM command to give an item.
            /// @param itemId Item id (0 asks to choose one first).
            /// @param count Amount.
            void give(int itemId, int count);

            /// @brief Tints the cell of the chosen item.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintChosen(gfx::Canvas& canvas, core::Point at);

            /// @brief Gives the category of an item from its Item.dat type (no item ids).
            /// @param item Item record.
            /// @return The category; Other for a type no category names.
            static Kind kindOf(const formats::ItemRecord& item);

            static constexpr int kColumns = 9; // the grid's Columns in AdminItemsWindow.xml
            static constexpr int kRows = 8; // the grid's Rows in AdminItemsWindow.xml

            GameServices& m_services;
            ui::Screen& m_screen;
            std::function<void(const std::string&)> m_sendCommand;
            ui::Window& m_window;
            ui::IconGrid& m_grid;
            ui::ScrollBar& m_scroll;
            const ui::Spot& m_chosenSpot;
            std::vector<int> m_allIds;
            std::vector<int> m_shown;
            Kind m_kind = Kind::All;
            int m_topRow = 0;
            int m_chosen = 0;
            bool m_adminMode = false;
    };

}

#endif
