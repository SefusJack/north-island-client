#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_ITEM_TRADE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_ITEM_TRADE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/items/OfferGrid.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/ItemInstance.h"
#include "protocol/TradeOffer.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Item trade window for one trade with a player on the map: own offer, partner's offer, Confirm, Trade, Cancel.
    /// @details Original: ItemTradeWindow.xml, TRe_TradeItemForm; partner's offer 25/3, Confirm 25/2, Trade 25/3 1, Cancel 25/3 2,
    /// how a trade ended 25/2.
    class ItemTradeWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory The player's inventory.
            /// @param marks The bag cells' marks (the offered items are marked in use).
            /// @param status The player's status (gold).
            /// @param messages Player message line for refusals and results.
            /// @param bagChanged Called when the bag cells' marks changed.
            ItemTradeWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                const PlayerStatus& status,
                PlayerMessages& messages,
                std::function<void()> bagChanged
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window for a trade with a partner.
            /// @details Original: StartTradeWith / OpenTradeWindow.
            /// @param partnerName The partner's name.
            void open(const std::string& partnerName);

            /// @brief Cancels the trade: sends 25/3 2 and puts everything back.
            /// @details Original: CancelTrade.
            void cancel();

            /// @brief Shows how a trade ended and closes the window.
            /// @details Original: RecvTradeResult (25/2).
            /// @param result The server's result code.
            void showResult(int result);

            /// @brief Shows the partner's offer.
            /// @details Original: RecvPartnerTradeItems (25/3).
            /// @param offer The partner's gold and items.
            void showPartnerOffer(const protocol::TradeOffer& offer);

            /// @brief Redraws the bag, both offers and the buttons.
            /// @details Original: RefreshGridItems, SyncMyOfferImages, SyncPartnerOfferImages; DrawControl's buttons.
            void refresh();

        private:
            /// @brief One item of the own offer: the bag slot it comes from and a copy of the item.
            struct Offered {
                int bagSlot = 0;
                protocol::ItemInstance item;
            };

            /// @brief Puts a bag item into the own offer, after the refusal checks.
            /// @details Original: SelectTradeBagItem and HandleBagItemClickForTrade, AddBagItemToOffer, PlaceItemInOffer.
            /// @param bagSlot Bag slot clicked.
            void offerFromBag(int bagSlot);

            /// @brief Takes an item back out of the own offer.
            /// @details Original: HandleOfferItemClick / RemoveOfferItem.
            /// @param cell Offer grid cell clicked.
            void takeBack(int cell);

            /// @brief Confirms the own offer (gold and items) to the server.
            /// @details Original: ConfirmTradeOffer (25/2).
            void confirmOffer();

            /// @brief Agrees to the trade (Trade button).
            /// @details Original: ConfirmTradeFinal (25/3 1).
            void agree();

            /// @brief Whether the partner's items find room in the bag.
            /// @details Original: CanReceiveTradeItems.
            /// @return True when every item fits.
            bool bagCanTake() const;

            /// @brief Bag cells an item occupies.
            /// @param itemId Item id.
            /// @return Width and height in cells.
            core::Size cellsOf(int itemId) const;

            /// @brief Shows the Confirm and Trade buttons that match the trade state.
            void layoutButtons();

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            std::function<void()> m_bagChanged;
            ui::Window& m_window;
            ui::IconGrid& m_bag;
            ui::IconGrid& m_mine;
            ui::IconGrid& m_partner;
            ui::TextBox& m_giveGold;
            ui::TextBox& m_partnerGold;
            OfferGrid m_myGrid;
            OfferGrid m_partnerGrid;
            std::array<Offered, OfferGrid::kSlots + 1> m_myOffer{};
            std::array<protocol::ItemInstance, OfferGrid::kSlots + 1> m_partnerOffer{};
            int m_offered = 0;
            bool m_offerConfirmed = false;
            bool m_partnerOfferReceived = false;
            bool m_agreed = false;
    };

}

#endif
