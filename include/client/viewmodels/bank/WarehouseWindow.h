#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_WAREHOUSE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BANK_WAREHOUSE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/BagItemRecord.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief City warehouse window: the warehouse and the bag side by side, items chosen and moved with the arrows.
    /// @details Original: WarehouseWindow.xml, TCY_WareMenu; TPlayer.FWareItems (50 places); packets 23/34 (deposit) and 23/35 (withdraw).
    class WarehouseWindow {
        public:
            /// @brief Builds the window and wires its grids and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param bag The player's bag.
            /// @param messages Player message sink.
            WarehouseWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& bag,
                PlayerMessages& messages
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window with nothing chosen.
            /// @details Original: OpenWareMenu; onClosed runs CloseWareMenu.
            /// @param onClosed Called when the window closes.
            void show(std::function<void()> onClosed);

            /// @brief Adds an item to the warehouse, into its place or (place 0) onto a non-full stack or the first empty place.
            /// @details Original: RecvWareItemList (given place) and RecvWareItemAdd (place 0).
            /// @param record Item and place.
            void add(const protocol::BagItemRecord& record);

            /// @brief Empties a warehouse place.
            /// @details Original: RecvWareSlotClear.
            /// @param slot Warehouse slot.
            void clear(int slot);

            /// @brief Gives the item in a warehouse place.
            /// @param slot Warehouse slot.
            /// @return The item there.
            const protocol::ItemInstance& item(int slot) const { return m_items.at(static_cast<std::size_t>(slot)); }

            /// @brief Turns every item id from first to last in the warehouse into one id.
            /// @details Original: RevertItemVariantsTo20747.
            /// @param first First id of the range.
            /// @param last Last id of the range.
            /// @param to Id they become.
            void replaceItemIds(int first, int last, int to);

            /// @brief Replaces what lies in a place by the item that came of it.
            /// @details Original: RecvThingExpired 4.
            /// @param slot Warehouse slot.
            /// @param item The new item.
            void replace(int slot, const protocol::ItemInstance& item);

            /// @brief Redraws both grids after the bag or warehouse changed.
            /// @details Original: UpdateWareItemHints.
            void refresh();

        private:
            /// @brief Toggles the choice of a warehouse place.
            /// @details Original: TCY_WareImage.HandleMouseDown.
            /// @param slot Warehouse slot.
            void pressItem(int slot);

            /// @brief Toggles the choice of a bag place; refused items say so.
            /// @details Original: TCY_WareImage.HandleMouseDown.
            /// @param slot Bag slot.
            void pressBag(int slot);

            /// @brief Sends the chosen bag items to the warehouse.
            /// @details Original: WareDepositClick (23/34).
            void deposit();

            /// @brief Takes the chosen warehouse items into the bag.
            /// @details Original: WareWithdrawClick (23/35).
            void withdraw();

            /// @brief Closes the window.
            void close();

            /// @brief Paints a tint over the chosen places.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paintChosen(gfx::Canvas& canvas, core::Point at);

            static constexpr int kSlots = 50; // warehouse slots, 1-based

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_bag;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::IconGrid& m_itemGrid;
            ui::IconGrid& m_bagGrid;
            const ui::Spot& m_chosenSpot;
            std::array<protocol::ItemInstance, kSlots + 1> m_items{};
            std::array<bool, kSlots + 1> m_itemChosen{};
            std::array<bool, kSlots + 1> m_bagChosen{};
            std::function<void()> m_onClosed;
    };

}

#endif
