#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_STALL_PRICE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_STALL_PRICE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The price form of the shelves, vending machines and display cases: the price of a bag item dropped on a piece of furniture.
    /// @details Original: StallPriceWindow.xml, TH_Menu.pas TTH_StallItemInFurForm; Confirm asks the server to put the item on the shelf (62/21), into the vending machine (62/24) or into a new case of the display case (62/58); for a case that is there it sets a new price.
    class StallPriceWindow {
        public:
            /// @brief What the price is for.
            /// @details Original: FOpenMode; a display case's is 2, with FRepriceMode 0 for an item of the bag (Case) and 1 for the case's own price (CaseReprice).
            enum class Mode { Shelf = 0, Vending = 1, Case = 2, CaseReprice = 3 };

            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<bool(int)> storable;
                std::function<bool()> caseFree;
                std::function<void(int, int, std::int32_t)> putInCase;
                std::function<void(std::int32_t)> repriceCase;
            };

            /// @brief Loads the form and wires its buttons.
            /// @param services Game services (layouts).
            /// @param screen Screen the form is loaded into.
            /// @param marks Bag cell marks.
            /// @param messages Sink for player notices.
            /// @param hooks Owner callbacks.
            StallPriceWindow(
                GameServices& services,
                ui::Screen& screen,
                const BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form with a price of 0 for an item of a bag place and a piece.
            /// @param mode What the price is for.
            /// @param furnIndex Index of the furniture piece.
            /// @param itemId Item id.
            /// @param bagSlot Bag slot of the item.
            /// @param count How many of the item.
            /// @param vendingRow Row of the vending machine the item goes to (0: the first free one).
            /// @details Original: ShowCtrl.
            void open(
                Mode mode,
                int furnIndex,
                int itemId,
                int bagSlot,
                int count,
                int vendingRow
            );

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Closes the form when it was opened for a case (the display case's form went).
            void closeForCase();

            /// @brief Opens the form to change the price of the shown case.
            /// @param furnIndex Index of the furniture piece.
            /// @param price The case's current price.
            /// @details Original: OpenStallCasePrice.
            void openReprice(int furnIndex, std::int32_t price);

        private:
            /// @brief Checks the price and the item and sends the request for the current mode.
            /// @details Original: ConfirmStallPrice (62/21 shelf, 62/24 vending machine, 62/58 display case).
            void confirm();

            /// @brief Changes the price by a step.
            /// @param by Amount to add (negative to lower).
            /// @details Original: DecStallPrice / IncStallPrice.
            void step(int by);

            /// @brief Reads the price from the text box.
            /// @return The price, 0 when the box is empty.
            int price() const;

            /// @brief Per-frame check: closes the form out of the house and lifts a negative price to 0.
            /// @details Original: DrawControl.
            void paint();

            ui::Screen& m_screen;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_price;
            Mode m_mode = Mode::Shelf;
            int m_furnIndex = 0;
            int m_itemId = 0;
            int m_bagSlot = 0;
            int m_count = 0;
            int m_vendingRow = 0;
    };

}

#endif
