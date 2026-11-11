#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_INSTEAD_ECIS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_INSTEAD_ECIS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/items/PuppetState.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "protocol/PuppetPacket.h"
#include "ui/CheckBox.h"
#include "ui/ImageButton.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief Memory Card window for having someone play for the player.
    /// @details Original: InsteadEcisWindow.xml, CJ_Menu.pas TInsteadEcisForm; the Memory Card 34232 "Used to restore EXP".
    /// Shows the card's exp and size, a friend or guild member to ask (typed or picked; OK sends 23/104 option 1) and, once the
    /// other plays, his portrait with buttons to check the time, write to him and stop. Exp is spent with Use, the card grows with Expand.
    class InsteadEcisWindow {
        public:
            /// @brief Callbacks into the rest of the game.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFight;
                std::function<int()> ownLevel;
                std::function<std::int32_t()> ownId;
                std::function<std::string()> ownName;
                std::function<bool()> enoughOnlineTime;
                std::function<std::optional<std::int32_t>(const std::string&)> playerNamed;
                std::function<bool(std::int32_t)> playerKnown;
                std::function<std::optional<protocol::PuppetPerson>(std::int32_t)> worldPerson;
                std::function<void(std::int32_t, const std::string&, std::function<void()>, std::function<void()>, std::function<void()>)> invited;
                std::function<void(const std::string&, std::function<void()>)> ask;
                std::function<void(const std::string&, std::function<void()>)> askExpand;
                std::function<void(const protocol::PuppetPerson&)> mailTo;
                std::function<void(std::int32_t)> whisper;
            };

            static constexpr int kMark = BagMarks::kMemoryCard;
            static constexpr int kBusyKind = 5; // busy-mark kind this window puts on a bag cell
            static constexpr int kMemoryCard = 34232; // item 34232: Memory Card (type 25)
            static constexpr int kStrongScroll = 30101; // item 30101: Strong Scroll (type 21)

            /// @brief Loads the window and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param friends Friend list.
            /// @param guild Guild member list.
            /// @param inventory Player inventory (read only).
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param state Puppet training state shared with other forms.
            /// @param hooks Callbacks into the rest of the game.
            InsteadEcisWindow(
                GameServices& services,
                ui::Screen& screen,
                const FriendList& friends,
                const Guild& guild,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                PuppetState& state,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for the Memory Card at a bag place.
            /// @details Original: UseMemoryCardItem (Se_EquipUI: "Unable to use" while training or open).
            /// @param bagSlot Bag place of the card.
            void use(int bagSlot);

            /// @brief Shows the form for a requester and asks the server for the partner.
            /// @details Original: ShowPuppetTrainForm for a requester; 23/104 option 12 asks the server for the partner.
            void showTraining();

            /// @brief Hides the form and frees the card's cell unless a request waits.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Types a player clicked in the world into the name field while the form is up.
            /// @details Original: SetWhisperTarget.
            /// @param name Player name.
            void setName(const std::string& name);

            /// @brief Handles the server's puppet packet for this form.
            /// @details Original: server 23/104 options 1, 2, 3, 4, 7, 12, 14, 15, 16, 18, 19, 20, 21 and 22.
            /// @param packet Decoded packet.
            void received(const protocol::PuppetPacket& packet);

            /// @brief Ends the training for the requester.
            /// @details Original: RecvInsteadEcisEnd.
            /// @param bagSlot Bag place of the card.
            void end(int bagSlot);

            /// @brief Clears the partner's name, id and online state.
            /// @details Original: AcceptCancelRequest.
            void clearPartner();

            /// @brief Resets exp, bar, partner and buttons.
            /// @details Original: ResetInsteadEcisForm.
            void reset();

            /// @brief The partner (V[12]), for the mail form.
            /// @return The partner's person record.
            const protocol::PuppetPerson& partner() const { return m_partner; }

        private:
            /// @brief Fills the name list with the ticked friends and guild members.
            /// @details Original: RefreshTrainPartnerList.
            void fillNames();

            /// @brief Puts the double-clicked name into the name field if the player is online.
            /// @details Original: PickTrainPartner.
            /// @param row List row.
            void pick(int row);

            /// @brief Checks the conditions and sends the request for someone to play.
            /// @details Original: SendPuppetTrainRequest (23/104 option 1).
            void invite();

            /// @brief Handles an invitation to help play (ask, accept, refuse, timeout).
            /// @details Original: RecvHelpTrainInvite, AcceptHelpTrainInvite, RefuseHelpTrainInvite and HelpTrainInviteTimeout.
            /// @param packet Decoded packet.
            void invitation(const protocol::PuppetPacket& packet);

            /// @brief Sets the helper from the packet and marks the card busy.
            /// @details Original: RecvInsteadEcisPartner.
            /// @param packet Decoded packet.
            void partnerSet(const protocol::PuppetPacket& packet);

            /// @brief Sets the partner and exp from the answer to opening the form from the status icon.
            /// @details Original: RecvInsteadEcisInfo.
            /// @param packet Decoded packet.
            void partnerInfo(const protocol::PuppetPacket& packet);

            /// @brief Shows or hides the buttons that need a partner.
            /// @param partnered True when a training with a partner is on.
            void showPartnerControls(bool partnered);

            /// @brief Asks whether to grow the card.
            /// @details Original: AskExpandCardCapacity.
            void askExpand();

            /// @brief Checks the Strong Scrolls and sends the grow request.
            /// @details Original: ConfirmExpandCardCapacity.
            void confirmExpand();

            /// @brief Asks whether to use the card's exp.
            /// @details Original: AskUseCardExp.
            void askUseExp();

            /// @brief Sends the request to use the card's exp.
            /// @details Original: SendUseCardExp.
            void useExp();

            /// @brief Asks whether to cancel the play-for-you.
            /// @details Original: AskCancelHelpPlay.
            void askCancel();

            /// @brief Asks the server to stop the training.
            /// @details Original: SendCancelHelpPlay.
            void cancel();

            /// @brief Asks the server for the time left.
            /// @details Original: RequestTrainTimeLeft.
            void askTimeLeft();

            /// @brief Shows the training time left.
            /// @details Original: RecvTrainTimeLeft (the requester's side).
            /// @param minutes Minutes left.
            void timeLeft(int minutes);

            /// @brief Shows the text of a training result code.
            /// @details Original: RecvPuppetTrainResult.
            /// @param code Result code.
            void result(int code);

            /// @brief Recomputes the exp bar width (B[44]) and the digits of the exp (B[45]).
            void sizeBar();

            /// @brief Frees a card's marks after the server released it.
            /// @details Original: RecvMemCardItemReleased.
            /// @param bagSlot Bag place of the card.
            void released(int bagSlot);

            /// @brief Marks the card's cell as busy with puppet training.
            /// @param bagSlot Bag place of the card.
            void markBusy(int bagSlot);

            /// @brief Frees the use mark the form holds on the card's cell.
            void releaseMark();

            /// @brief Stores the partner and builds his look.
            /// @param person The partner.
            void setPartner(const protocol::PuppetPerson& person);

            /// @brief Draws the exp bar and numbers, the partner's name, status and portrait.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds (portrait blink).
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            const FriendList& m_friends;
            const Guild& m_guild;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            PuppetState& m_state;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ListBox& m_names;
            ui::TextBox& m_name;
            ui::CheckBox& m_friendsTick;
            ui::CheckBox& m_guildTick;
            ui::ImageButton& m_useButton;
            ui::ImageButton& m_expandButton;
            ui::ImageButton& m_cancelButton;
            ui::ImageButton& m_timeButton;
            ui::Element& m_mailButton;
            const ui::Spot& m_barSpot;
            const ui::Spot& m_expSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_onlineSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_textSpot;
            int m_bagSlot = 0;
            std::int64_t m_exp = 0;
            std::int64_t m_capacity = 0;
            int m_barWidth = 0;
            int m_digits = 1;
            protocol::PuppetPerson m_partner;
            bool m_partnerOnline = false;
            CharacterLook m_partnerLook;
            bool m_requestSent = false;
            PortraitBlink m_blink;
    };

}

#endif
