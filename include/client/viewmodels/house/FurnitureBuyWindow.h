#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_FURNITURE_BUY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_FURNITURE_BUY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/house/StallCaseChange.h"
#include "client/models/house/StallInFurItem.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The buy form for an item on a shelf, in a vending machine or in a display case (FurnitureBuyWindow.xml, TTH_BuyitemFOrm).
    /// @details Original: TH_Menu.pas TTH_BuyitemFOrm. Shows its name and price, how many are left, how many to take and what they come to. OK asks the server (62/23 from a shelf, 62/26 from a vending machine, 62/59 from a display case).
    class FurnitureBuyWindow {
        public:
            /// @brief Where the item lies.
            /// @details Original: FShopKind.
            enum class Kind { Shelf = 0, Vending = 1, Case = 2 };

            /// @brief Callbacks into the rest of the client that the buy form needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<std::int32_t()> gold;
            };

            /// @brief Loads the window layout and wires the OK and close buttons.
            /// @param services Game services (layouts).
            /// @param screen Screen the window is loaded into.
            /// @param house The house interior with its utensils and stalls.
            /// @param inventory The player's bag.
            /// @param messages Player message line for refusals.
            /// @param hooks Callbacks into the rest of the client.
            FurnitureBuyWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const Inventory& inventory,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form for an item of a shelf, a vending machine or a display case, with a count of 0.
            /// @details Original: ShowCtrl, after the callers set the fields.
            /// @param kind Where the item lies.
            /// @param item The stall item.
            /// @param name The name to show (the fusion level of gear in it).
            void open(Kind kind, const StallInFurItem& item, const std::string& name);

            /// @brief Tells the buyer when the seller took back the shown item (62/22); the window goes.
            /// @param furnIndex Furniture piece index of the item.
            /// @param posInFurn Place of the item in the piece.
            void reclaimed(int furnIndex, int posInFurn);

            /// @brief Handles a display case update for a buyer: a price or count that changed under him, or a case taken away.
            /// @details Original: RecvStallCaseUpdate.
            /// @param change The before and after state of the case item.
            void caseChanged(const StallCaseChange& change);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Checks bag room and gold, then sends the buy request (62/23 shelf, 62/26 vending machine, 62/59 display case) and closes the form.
            /// @details Original: ConfirmBuyHomeItem.
            void confirm();

            /// @brief Reads the typed count.
            /// @return The count, 0 for an empty field.
            int count() const;

            /// @brief Paints the price, the name, how many are left and, once a number is typed, the total; keeps the count in range.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_count;
            const ui::Spot& m_priceSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_leftSpot;
            const ui::Spot& m_totalSpot;
            Kind m_kind = Kind::Shelf;
            std::string m_name;
            int m_furnIndex = 0;
            int m_itemCount = 0;
            std::int32_t m_unitPrice = 0;
            int m_goodsSlot = 0;
            int m_stallIndex = 0;
    };

}

#endif
