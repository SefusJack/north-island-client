#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_EGG_SELL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_EGG_SELL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief A Fortune Egg machine seen by its owner (EggSellWindow.xml, TTH_SellEggMachineForm): the ten places, the price of a play and the eggs it holds.
    /// @details Original: TH_Menu.pas TTH_SellEggMachineForm. Bag stacks of at least ten are dropped on a place (62/31), a right click takes one back (62/35), Setup sets the price after a question (62/32) and Stop stops selling.
    class EggSellWindow {
        public:
            /// @brief Callbacks into the rest of the client that the egg sell window needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<void(const std::string&, std::function<void()>)> askTimed;
            };

            /// @brief Loads the window layout and wires the paint area and buttons.
            /// @param services Game services (layouts, item records).
            /// @param screen Screen the window is loaded into.
            /// @param house The house interior with its utensils and stalls.
            /// @param inventory The player's bag.
            /// @param marks Bag slot marks (locks).
            /// @param messages Player message line for refusals.
            /// @param hooks Callbacks into the rest of the client.
            EggSellWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the machine of a piece with the price it has.
            /// @details Original: ShowCtrl.
            /// @param furnIndex Furniture piece index of the machine.
            void open(int furnIndex);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Handles a bag item dropped on a place of the machine: the checks in the original's order, then the put request (62/31).
            /// @details Original: HandleItemDrop on a place of the machine (FSlotKind 12).
            /// @param bagSlot Bag slot of the dropped item.
            /// @param place Machine place (1..10) it was dropped on.
            void drop(int bagSlot, int place);

            /// @brief Handles a price change of the machine (62/32).
            /// @param furnIndex Furniture piece index of the machine.
            /// @param price New price of a play.
            void priceChanged(int furnIndex, std::int32_t price);

        private:
            /// @brief Checks the machine and the typed price, then asks the question that confirms it.
            /// @details Original: SetEggPriceClick.
            void setPrice();

            /// @brief Repeats the checks, then sends the price (62/32) and closes the form.
            /// @details Original: ConfirmEggPrice.
            void confirmPrice();

            /// @brief Stops selling by sending a price of 0 (62/32).
            /// @details Original: StopEggSelling.
            void stopSelling();

            /// @brief Takes the stack of a place back to the bag (right click, 62/35).
            /// @details Original: RetakeEggItem.
            /// @param place Machine place (1..10).
            void retake(int place);

            /// @brief Finds the place of the machine with nothing in it.
            /// @details Original: FindEmptyEggSlot.
            /// @return The place number, 0 for none.
            int emptyPlace() const;

            /// @brief Tells whether every place of the machine holds something.
            /// @return True when no place is empty.
            bool allPlacesFull() const;

            /// @brief Tells whether a stack of fewer than ten lies in the machine.
            /// @return True when there is such a stack.
            bool hasShortStack() const;

            /// @brief Reads the typed price.
            /// @return The price, 0 for an empty field.
            int price() const;

            /// @brief Gives the house utensil of the shown machine.
            /// @return The machine's utensil record.
            const HouseUtensil& utensil() const;

            /// @brief Paints the egg count and fills the icon cells with the stacks in the machine's places; closes the form out of the house.
            /// @details Original: DrawControl: the eggs the machine holds (0 when it is not selling) and the stacks in its places.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_price;
            ui::IconGrid& m_cells;
            const ui::Spot& m_eggCountSpot;
            const ui::Spot& m_noEggsSpot;
            int m_furnIndex = 0;
    };

}

#endif
