#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_POINT_STORE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_POINT_STORE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/menus/CountBox.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/BlockList.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Window of an event's point store: pick items into a purchase list of ten places and buy them with IM points.
    /// @details Original: PointStoreWindow.xml, MZ_Menu.pas TEightTwelveForm; the rows come with 80/1, the server answers each line with 80/3.
    class PointStoreWindow {
        public:
            /// @brief Callbacks into the rest of the game that the window needs.
            struct Hooks {
                std::function<void(const std::string&, std::function<void()>)> ask;
                std::function<void(int channel, const std::string&)> chat;
                std::function<void()> askPoints;
                std::function<void()> closed;
                std::function<bool()> questionOpen;
            };

            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param inventory Own inventory, used to tell stackable items.
            /// @param messages Sink for short player messages.
            /// @param countBox Box that asks how many of a stackable item.
            /// @param hooks Callbacks into the game.
            PointStoreWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                PlayerMessages& messages,
                CountBox& countBox,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Handles a server action 80 packet.
            /// @details Original: RecvPointStoreList, RecvPointStoreBought, RecvPointStoreBuyResult, ShowPointsDeducted, RecvPointsDeducted and the notices (80/1 to 80/8).
            /// @param payload The whole message; byte 1 is the sub id.
            void receive(const core::Bytes& payload);

            /// @brief Takes the account's IM points while the form is open and has none yet.
            /// @details Original: 35/4.
            /// @param points The account's IM points.
            void pointsArrived(std::int32_t points);

        private:
            /// @brief One row of the list: the item, its price and how many it gives (more than one: a pack).
            struct Row {
                int itemId = 0;
                std::int32_t price = 0;
                int count = 0;
            };

            /// @brief One of the ten places of the purchase list.
            /// @details Original: FCartRows, FCartItems.
            struct CartLine {
                int row = 0;
                int itemId = 0;
                std::int32_t unitPrice = 0;
                bool single = false;
                int count = 0;
            };

            static constexpr int kCartSlots = 10; // shopping cart slots, 1-based

            /// @brief Brings the form up with an empty list and asks for the points.
            /// @details Original: ShowCtrl.
            void show();

            /// @brief Closes the form and the count box and tells the event it went.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Empties the purchase list.
            /// @details Original: ClearPointStoreCart, ResetPointStoreCart.
            void clearCart();

            /// @brief Puts a clicked row into the purchase list.
            /// @details Original: PickPointStoreItem.
            /// @param row Row index from 0.
            /// @param askCount True (Ctrl) to ask how many of a stackable item.
            void pick(int row, bool askCount);

            /// @brief Adds an item to a place of the purchase list and its price to the total.
            /// @details Original: AddPointStoreCartItem.
            /// @param rowNumber Row number from 1.
            /// @param itemId Item id.
            /// @param count How many.
            /// @param unitPrice Price in IM points.
            /// @param single True for a pack, which costs its price once.
            void addToCart(
                int rowNumber,
                int itemId,
                int count,
                std::int32_t unitPrice,
                bool single
            );

            /// @brief Finds a free place of the purchase list.
            /// @details Original: FindFreePointStoreCartSlot.
            /// @return Place from 1, or 0 for none.
            int freeSlot() const;

            /// @brief Finds the place for an item, joining an equal item while it stays within 50.
            /// @details Original: FindPointStoreCartSlotFor.
            /// @param itemId Item id.
            /// @param count How many.
            /// @return Place from 1, or 0 for none.
            int slotFor(int itemId, int count) const;

            /// @brief Takes an item out of the purchase list.
            /// @details Original: RemovePointStoreCartItem.
            /// @param slot Place from 1.
            void takeBack(int slot);

            /// @brief Asks the player to confirm the purchase.
            /// @details Original: ConfirmPointStoreBuy.
            void buy();

            /// @brief Sends the purchase list (80/1) and empties it.
            /// @details Original: SendPointStoreBuy.
            void send();

            /// @brief Brings the form up and adds the rows of 80/1.
            /// @details Original: RecvPointStoreList.
            /// @param payload The whole message.
            void receiveRows(const core::Bytes& payload);

            /// @brief Adds one row to the list.
            /// @details Original: AddPointStoreRow.
            /// @param itemId Item id.
            /// @param price Price in IM points.
            /// @param count How many the row gives.
            void addRow(int itemId, std::int32_t price, int count);

            /// @brief Redraws the icons of the purchase list.
            void refreshCart();

            /// @brief Paints the cost of the purchase list and the points the account has.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top left of the paint area on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            CountBox& m_countBox;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ImageButton& m_buyButton;
            ui::BlockList& m_list;
            ui::IconGrid& m_cart;
            const ui::Spot& m_totalSpot;
            const ui::Spot& m_pointsSpot;
            const ui::Spot& m_textColorSpot;
            std::vector<Row> m_rows;
            std::array<CartLine, kCartSlots + 1> m_lines{};
            int m_cartCount = 0;
            std::int32_t m_total = 0;
            std::int32_t m_points = 0;
            int m_pickedRow = 0;
            std::int32_t m_boughtPoints = 0;
    };

}

#endif
