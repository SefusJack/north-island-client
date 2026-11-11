#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_MEM_CARD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_MEM_CARD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/models/items/PuppetState.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "protocol/PuppetPacket.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The Memory Card form of the helper who plays for others: up to three cards with the player, exp bar and buttons.
    /// @details Original: MemCardWindow.xml, CJ_Menu.pas TMemCardForm1.
    class MemCardWindow {
        public:
            /// @brief Callbacks into the surrounding scene.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFight;
                std::function<std::int32_t()> ownId;
                std::function<bool(std::int32_t)> playerKnown;
                std::function<std::optional<protocol::PuppetPerson>(std::int32_t)> worldPerson;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>, std::function<void()>)> request;
                std::function<void(const std::string&, std::function<void()>)> ask;
                std::function<void(const protocol::PuppetPerson&)> mailTo;
                std::function<void(std::int32_t)> whisper;
                std::function<void()> clearPartner;
            };

            static constexpr int kCards = 3;

            /// @brief Builds the form and wires its controls.
            /// @param services Shared game services.
            /// @param screen Screen the form lives on.
            /// @param friends Friend list.
            /// @param guild Guild member list.
            /// @param messages Player message sink.
            /// @param state Puppet training state.
            /// @param hooks Scene callbacks.
            MemCardWindow(
                GameServices& services,
                ui::Screen& screen,
                const FriendList& friends,
                const Guild& guild,
                PlayerMessages& messages,
                PuppetState& state,
                Hooks hooks
            );

            /// @brief Whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form on top.
            /// @details Original: ShowPuppetTrainForm for a helper.
            void show();

            /// @brief Hides the form.
            void close();

            /// @brief Empties the cards and hides their buttons.
            /// @details Original: ResetMemCardForm.
            void reset();

            /// @brief Handles a server 23/104 packet for this form.
            /// @param packet The packet; its option is one of 5, 8, 9, 10, 13, 15, 16, 17, 21.
            /// @details Original: server 23/104.
            void received(const protocol::PuppetPacket& packet);

        private:
            /// @brief One of the three cards: the player, his exp and the bar sizes.
            /// @details Original: FCardHumans, FCardSlots, FBarUnits, FBarWidths.
            struct Card {
                protocol::PuppetPerson person;
                std::int64_t exp = 0;
                bool online = false;
                int step = 0;
                int barUnit = 0;
                int barWidth = 0;
                CharacterLook look;
            };

            /// @brief Sets one card of the helper from a packet.
            /// @param packet Server packet (option 13).
            /// @details Original: RecvMemCardSlot.
            void cardSet(const protocol::PuppetPacket& packet);

            /// @brief The exp of a card changed; a longer packet also sets the person.
            /// @param packet Server packet (option 8).
            /// @details Original: RecvMemCardSlotExp.
            void cardExp(const protocol::PuppetPacket& packet);

            /// @brief Empties a card.
            /// @param slot Card number 1..3.
            /// @details Original: RecvMemCardSlotCleared.
            void cardCleared(int slot);

            /// @brief Sets the card of a friend or guild member; its person comes from the lists.
            /// @param packet Server packet (option 17).
            /// @details Original: RecvMemCardSlotById.
            void cardById(const protocol::PuppetPacket& packet);

            /// @brief The card the server numbers 1..3.
            /// @param slot Card number.
            /// @return The card, or none when out of range.
            std::optional<std::reference_wrapper<Card>> cardAt(int slot);

            /// @brief Shows or hides the buttons of a card.
            /// @param place Card place 0..2.
            /// @param shown True to show.
            void showButtons(int place, bool shown);

            /// @brief Sets the person of a card and its look.
            /// @param card The card.
            /// @param person The person.
            void setPerson(Card& card, const protocol::PuppetPerson& person);

            /// @brief Sizes a card's bar: the step gives the unit, the exp the width.
            /// @param card The card.
            void sizeBar(Card& card);

            /// @brief Asks to stop the training of a card's player.
            /// @param place Card place 0..2.
            /// @details Original: AskCancelTraining.
            void askCancel(int place);

            /// @brief Sends the stop request after the question was confirmed.
            /// @details Original: SendCancelTraining.
            void cancel();

            /// @brief The requester wants to stop the helper's playing for him.
            /// @param packet Server packet (option 5).
            /// @details Original: RecvMemCardCancelRequest.
            void stopWanted(const protocol::PuppetPacket& packet);

            /// @brief The helper wants to stop (told to the requester).
            /// @param packet Server packet (option 10).
            /// @details Original: RecvCancelRequest.
            void helperStopWanted(const protocol::PuppetPacket& packet);

            /// @brief Refreshes what each card shows from the world and the lists.
            /// @details Original: RefreshMemCardSlots.
            void refresh();

            /// @brief Paints all cards and the exp hint.
            /// @param canvas Target canvas.
            /// @param at Top left of the paint area.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the exp hint over a card's bar under the mouse.
            /// @param canvas Target canvas.
            /// @param at Top left of the paint area.
            /// @details Original: DrawHintBox.
            void paintHint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints one card's row.
            /// @param canvas Target canvas.
            /// @param at Top left of the paint area.
            /// @param place Card place 0..2.
            /// @param nowMs Current time in ms.
            void paintCard(gfx::Canvas& canvas, core::Point at, int place, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            const FriendList& m_friends;
            const Guild& m_guild;
            PlayerMessages& m_messages;
            PuppetState& m_state;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::ImageButton>, kCards> m_mailButtons;
            std::array<std::reference_wrapper<ui::ImageButton>, kCards> m_stopButtons;
            std::array<std::reference_wrapper<ui::ImageButton>, kCards> m_timeButtons;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_jobSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_titleSpot;
            const ui::Spot& m_guildSpot;
            const ui::Spot& m_barSpot;
            const ui::Spot& m_levelWordSpot;
            const ui::Spot& m_levelDigitsSpot;
            const ui::Spot& m_onlineSpot;
            const ui::Spot& m_elementSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_hintFillSpot;
            const ui::Spot& m_hintBorderSpot;
            const ui::Spot& m_hintTextSpot;
            std::array<Card, kCards> m_cards;
            std::int32_t m_targetId = 0;
            PortraitBlink m_blink;
    };

}

#endif
