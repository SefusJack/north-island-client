#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_UTENSIL_TRADE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_UTENSIL_TRADE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/items/OfferGrid.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
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

    /// @brief Window for trading large items between two players on the map.
    /// @details Original: UtensilTradeWindow.xml, Se_EquipUI.pas TRe_TradeUtensilForm, 25/40 to 25/42; OK puts the offer up (25/41),
    /// Trade agrees (25/42 1), the server ends the trade (25/42).
    class UtensilTradeWindow {
        public:
            /// @brief Callbacks into player and scene state the window needs.
            struct Hooks {
                std::function<std::string()> ownName;
                std::function<bool()> ownStall;
                std::function<bool(std::int32_t)> partnerStall;
                std::function<bool()> onVehicle;
                std::function<bool()> sportActive;
                std::function<void()> bagChanged;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory Player's bag.
            /// @param marks Bag cell marks (in-use, locks, vehicle state).
            /// @param status Player status (gold).
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Callbacks into player and scene state.
            UtensilTradeWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                const PlayerStatus& status,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Starts the trade over and shows the window; the partner comes with setPartner.
            /// @details Original: RecvUtensilTrade 40 (25/40): ShowCtrl (ResetUtensilTrade).
            void open();

            /// @brief Runs the stall checks and shows the partner's name.
            /// @details Original: RecvUtensilTrade 40 (25/40) after the player is found.
            /// @param partnerId Id of the partner player.
            /// @param name Partner's name.
            void setPartner(std::int32_t partnerId, const std::string& name);

            /// @brief Clears the trade marks in the bag and hides the window.
            /// @details Original: HideCtrl: ClearUtensilTradeMarks and the window goes.
            void close();

            /// @brief Asks the server to cancel the trade; the server answers with the end of the trade.
            /// @details Original: CancelUtensilTrade (25/42 2).
            void cancel();

            /// @brief Shows the partner's gold and goods.
            /// @details Original: RecvUtensilTrade 41 (25/41).
            /// @param offer The partner's offer.
            void showPartnerOffer(const protocol::TradeOffer& offer);

            /// @brief Shows why the trade ended and closes the window.
            /// @details Original: RecvUtensilTrade 42 (25/42).
            /// @param code Result code of the trade.
            void showResult(int code);

            /// @brief Describes the piece under the mouse, in the bag or among the partner's goods.
            void update();

            /// @brief Redraws the bag and partner cells and the button icons.
            /// @details Original: RefreshGridItems, SyncPartnerOfferImages; DrawControl's icons.
            void refresh();

        private:
            /// @brief Clears the offer state, the partner and the name and gold boxes.
            /// @details Original: ResetUtensilTrade.
            void reset();

            /// @brief Marks a bag piece as offered, or frees it when marked (only before OK).
            /// @details Original: ToggleUtensilOfferItem.
            /// @param bagSlot Bag slot (1-based).
            void toggle(int bagSlot);

            /// @brief Sends the gold and the marked pieces as the own offer.
            /// @details Original: ConfirmUtensilOffer (25/41).
            void confirmOffer();

            /// @brief Agrees to the trade.
            /// @details Original: ConfirmUtensilTrade (25/42 1).
            void agree();

            /// @brief Shows each button's icon where the button is hidden.
            void layoutButtons();

            /// @brief Applies the refusals both buttons share: a stall of either side, a vehicle of the player's.
            /// @return True when a refusal message was shown.
            bool refused();

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_bag;
            ui::IconGrid& m_partner;
            ui::TextBox& m_giveGold;
            ui::TextBox& m_partnerGold;
            ui::Element& m_okButton;
            ui::Element& m_tradeButton;
            ui::Element& m_confirmIcon;
            ui::Element& m_tradeIcon;
            OfferGrid m_partnerGrid{10}; // 10 rows x 5 columns (large-item grid)
            std::array<protocol::ItemInstance, 51> m_partnerOffer{}; // 5 x 10 slots + 1 (owner slots are 1-based)
            std::int32_t m_partnerId = 0;
            int m_offered = 0;
            bool m_requestSent = false;
    };

}

#endif
