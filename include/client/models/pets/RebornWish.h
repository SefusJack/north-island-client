#ifndef NORTH_ISLAND_CLIENT_MODELS_PETS_REBORN_WISH_H
#define NORTH_ISLAND_CLIENT_MODELS_PETS_REBORN_WISH_H

#include "client/services/GameServices.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "core/Bytes.h"
#include "formats/NpcInfo.h"

#include <array>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The Wishing God's wish form: lists wishable pets, asks the cost in stars and sends the wish.
    /// @details Original: RE_Menu.pas TRe_RebornForm (no look of its own, it is the talks it opens); packets 66/11 to 66/16;
    /// the speaker is the NPC 14108.
    class RebornWish {
        public:
            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<bool(int flagBit)> hasMarkFlag;
                std::function<bool()> petRoom;
                std::function<void(const std::string&, int)> message;
                std::function<void()> closed;
            };

            /// @brief Creates the wish form.
            /// @param services Game services (npc, talk and mark tables, connection).
            /// @param inventory The player's bag, to count stars.
            /// @param talk Talk dialog used for all the boxes.
            /// @param hooks Callbacks into the client.
            RebornWish(
                GameServices& services,
                const Inventory& inventory,
                TalkDialog& talk,
                Hooks hooks
            );

            /// @brief Handles a server message of the wish form.
            /// @details Original: RecvRebornWishList, RecvRebornWishIds, the form's ShowCtrl, RecvRebornWishRemoved, ShowRebornWishChoice
            /// and ShowRebornWishGreeting (packets 66/11 to 66/16).
            /// @param payload Message payload.
            void receive(const core::Bytes& payload);

            /// @brief Tells whether the form is up (original FIsVisible).
            /// @return True while open.
            bool isOpen() const { return m_open; }

        private:
            static constexpr int kListSize = 50; // places of the wish list

            /// @brief Gets the speaker of the form's talks.
            /// @return The Wishing God, or nothing when its NPC is unknown.
            std::optional<TalkDialog::Speaker> speaker() const;

            /// @brief Reads a wish list (original RecvRebornWishList / RecvRebornWishIds).
            /// @param payload Message payload.
            /// @param extra True for the list with 10 extra bytes after each key (66/11), false for keys only (66/14).
            void receiveList(const core::Bytes& payload, bool extra);

            /// @brief Removes a wished pet from the list (original RecvRebornWishRemoved, packet 66/13).
            /// @param payload Message payload.
            void receiveRemoved(const core::Bytes& payload);

            /// @brief Shows the pet pick box (original ShowRebornWishChoice).
            void showChoice();

            /// @brief Asks the cost in stars of the picked pet (original PickRebornWishPet).
            /// @param row Picked row of the list.
            void pick(int row);

            /// @brief Counts the stars and sends the wish when they cover the cost (original ConfirmRebornWish).
            /// @param choice 0 for Yes, otherwise No.
            void confirm(int choice);

            /// @brief Closes the form (original CloseRebornWish).
            void close();

            /// @brief Gives the price in stars of a pet: its grow key's, half (rounded up) with the flag of the mark of its career.
            /// @param npc The pet kind.
            /// @return The cost in stars.
            int cost(const formats::NpcInfo& npc) const;

            /// @brief Says a modal talk of the main box with the speaker.
            /// @param text Text to say.
            void say(const std::string& text);

            GameServices& m_services;
            const Inventory& m_inventory;
            TalkDialog& m_talk;
            Hooks m_hooks;
            std::array<int, kListSize + 1> m_wishes{};
            int m_wishIndex = 0;
            bool m_open = false;
    };

}

#endif
