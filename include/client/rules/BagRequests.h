#ifndef NORTH_ISLAND_CLIENT_RULES_BAG_REQUESTS_H
#define NORTH_ISLAND_CLIENT_RULES_BAG_REQUESTS_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/menus/CountBox.h"
#include "client/viewmodels/chat/PlayerMessages.h"

#include <cstddef>
#include <functional>
#include <string>

namespace client {

    /// @brief Rules for the player's mouse requests on bag items: move a stack, drop on the ground, destroy a worn item.
    /// @details Original: DZ_Thing.pas RequestMoveBagItem and RequestDropBagItem, Se_EquipUI.pas AskBagMoveCount and AskBagDropCount
    /// through the count box, DZ_Human.pas ConfirmDiscardEquip.
    class BagRequests {
        public:
            /// @brief What the rules need of the world.
            struct Hooks {
                std::function<bool()> swimming;
                std::function<std::size_t()> groundCount;
                std::function<int()> mapId;
                std::function<bool()> inHouseMap;
                std::function<bool()> fighting;
                std::function<int()> level;
                std::function<bool()> enoughOnlineTime;
                std::function<int()> campItemId;
                std::function<void(const std::string&, std::function<void()>)> confirm;
                std::function<int()> roleIndex;
                std::function<int(int kind)> bottlePool;
                std::function<protocol::ItemInstance(int petPlace, int slot)> petWorn;
            };

            /// @brief Builds the rules over the services, the bag, its marks, the message line, the count box and the world hooks.
            /// @param services Item table and connection.
            /// @param bag The player's inventory.
            /// @param marks Lock and vehicle marks of the bag slots.
            /// @param messages Player message line.
            /// @param countBox Box that asks for a count.
            /// @param hooks What the rules need of the world.
            BagRequests(
                GameServices& services,
                const Inventory& bag,
                const BagMarks& marks,
                PlayerMessages& messages,
                CountBox& countBox,
                Hooks hooks
            );

            /// @brief Moves a count of the items of a place into another; nothing moves unless that many fit, and a potion goes into the Bottle of its kind.
            /// @details Original: RequestMoveBagItem.
            /// @param fromSlot Source bag place.
            /// @param count How many to move.
            /// @param toSlot Destination bag place.
            void moveItem(int fromSlot, int count, int toSlot);

            /// @brief Drops a count of the items of a place on the ground, with the game's refusals (the house maps' rules are the house editor's).
            /// @details Original: RequestDropBagItem.
            /// @param slot Bag place.
            /// @param count How many to drop.
            void dropItem(int slot, int count);

            /// @brief Handles an item dropped on the ground: Ctrl asks how many, else one is dropped (a fight allows none; the birthday room no drop at all).
            /// @details Original: HandleItemDrop.
            /// @param slot Bag place.
            /// @param ctrl Whether Ctrl was held.
            void dropOnGround(int slot, bool ctrl);

            /// @brief Asks how many of the stack go to the other place.
            /// @details Original: AskBagMoveCount.
            /// @param fromSlot Source bag place.
            /// @param toSlot Destination bag place.
            void askMoveCount(int fromSlot, int toSlot);

            /// @brief Asks how many go on the ground.
            /// @details Original: AskBagDropCount.
            /// @param slot Bag place.
            void askDropCount(int slot);

            /// @brief Destroys the worn item in an equipment place (of the player, or of the pet in petPlace when that is not 0) after the questions the item calls for.
            /// @details Original: AskDiscardEquip.
            /// @param equipSlot Equipment slot.
            /// @param petPlace Pet place, or 0 for the player's own equipment.
            void askDiscardEquip(int equipSlot, int petPlace);

        private:
            /// @brief Destroys the worn item, asking once more for the newbie set.
            /// @details Original: OnDiscardEquipOk.
            /// @param equipSlot Equipment slot.
            void confirmDiscard(int equipSlot);

            GameServices& m_services;
            const Inventory& m_bag;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            CountBox& m_countBox;
            Hooks m_hooks;
    };

}

#endif
