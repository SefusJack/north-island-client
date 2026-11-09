#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_PET_TRADE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_PET_TRADE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/pets/PetInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/PetTradeOffer.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <optional>
#include <string>

namespace client {

    /// @brief Pet trade window for one trade with a player on the map: own pet and gold, partner's pet and gold, Confirm, Trade, Cancel.
    /// @details Original: PetTradeWindow.xml, TCY_TradeFNpcMenu; partner's pet and gold 25/12, Confirm 25/11, Trade 25/12 1,
    /// Cancel 25/12 2, how it ended 25/11.
    class PetTradeWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param pets The player's pet team.
            /// @param status The player's status (gold).
            /// @param messages Player message line for refusals and results.
            /// @param info Pet info popup shown over the partner's pet.
            PetTradeWindow(
                GameServices& services,
                ui::Screen& screen,
                const PetTeam& pets,
                const PlayerStatus& status,
                PlayerMessages& messages,
                PetInfoPopup& info
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief The place of the pet put up.
            /// @details Original: FOfferPetSlot.
            /// @return The pet team place, 0 for none.
            int offeredPlace() const { return m_place; }

            /// @brief Shows the window for a trade with a partner.
            /// @details Original: StartPetTradeWith / ShowCtrl.
            /// @param ownName The player's name.
            /// @param partnerName The partner's name.
            void open(const std::string& ownName, const std::string& partnerName);

            /// @brief Cancels the trade: sends 25/12 2 and hides the window.
            /// @details Original: CancelPetTrade.
            void cancel();

            /// @brief Shows how the trade ended and hides the window.
            /// @details Original: RecvPetTradeResult (25/11).
            /// @param result The server's result code.
            void showResult(int result);

            /// @brief Shows the partner's pet and gold.
            /// @details Original: RecvPartnerPetOffer (25/12).
            /// @param offer The partner's offer.
            void showPartnerOffer(const protocol::PetTradeOffer& offer);

        private:
            /// @brief Chooses the next or previous own pet.
            /// @details Original: SelectNextTradePet / SelectPrevTradePet; order 0 is none, else the pet of that join order.
            /// @param step Positive for next, otherwise previous.
            void choose(int step);

            /// @brief Shows the chosen own pet's name and gear.
            /// @details Original: SyncTradePetEquips; a pet that may not be traded is named in orange.
            void showOwnPet();

            /// @brief Confirms the own offer (pet and gold) to the server.
            /// @details Original: ConfirmPetTradeOffer (25/11).
            void confirmOffer();

            /// @brief Agrees to the trade (Trade button).
            /// @details Original: ConfirmPetTradeFinal (25/12 1).
            void agree();

            /// @brief The checks of an own pet both sides make.
            /// @param offering True when making the offer, false when agreeing.
            /// @return False, with the game's message shown, when the pet may not go.
            bool petMayGo(bool offering);

            /// @brief Shows the Confirm and Trade buttons that match the trade state.
            void layoutButtons();

            /// @brief Draws the gear backgrounds, both pets and the partner's pet info popup.
            /// @details Original: DrawControl, RenderWithChildren.
            /// @param canvas Canvas to draw on.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Draws one side's pet facing the viewer with its numbers.
            /// @param canvas Canvas to draw on.
            /// @param at Top-left of the drawing area.
            /// @param npc The pet's NPC info.
            /// @param feet Where the pet's feet stand, relative to the area.
            /// @param text Where the level text goes, relative to the area.
            /// @param level The pet's level.
            /// @param hp The pet's hp.
            /// @param maxHp The pet's maximum hp.
            /// @param amity The pet's amity.
            void paintPet(
                gfx::Canvas& canvas,
                core::Point at,
                const formats::NpcInfo& npc,
                core::Point feet,
                core::Point text,
                int level,
                std::int32_t hp,
                std::int32_t maxHp,
                int amity
            );

            /// @brief Hides the window and the pet info popup.
            void hide();

            GameServices& m_services;
            ui::Screen& m_screen;
            const PetTeam& m_pets;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            PetInfoPopup& m_info;
            ui::Window& m_window;
            ui::TextBox& m_giveGold;
            ui::TextBox& m_partnerGold;
            ui::TextBox& m_myPetName;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_fixedNameSpot;
            int m_order = 0;
            int m_place = 0;
            std::optional<protocol::PetTradeOffer> m_partner;
            bool m_offerConfirmed = false;
            bool m_agreed = false;
            bool m_infoShown = false;
    };

}

#endif
