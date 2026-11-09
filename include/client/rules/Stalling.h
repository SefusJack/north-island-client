#ifndef NORTH_ISLAND_CLIENT_RULES_STALLING_H
#define NORTH_ISLAND_CLIENT_RULES_STALLING_H

#include "formats/ItemTable.h"

namespace client {

    /// @brief Tells whether an item stall takes an item (CheckItemStallSell, AC_Item.pas).
    /// @param items Item table.
    /// @param itemId Id of the item.
    /// @return 0 when it can be sold, 1 when it may not change hands, 255 when not from a stall.
    int itemStallRefusal(const formats::ItemTable& items, int itemId);

    /// @brief Tells whether an NPC buys an item (CheckEquipShopSale / CheckItemShopSale, AC_Item.pas).
    /// @param items Item table.
    /// @param itemId Id of the item.
    /// @param equipmentShop True for an equipment NPC, false for a goods NPC.
    /// @return 0 when the NPC buys it, 1 when it may not be sold to NPCs, 255 when not that NPC's kind of item.
    int npcSaleRefusal(const formats::ItemTable& items, int itemId, bool equipmentShop);

    /// @brief Tells whether an item is one of the crystals that may never change hands.
    /// @details Original: IsNonTransferableItem (NonTransferableItemTable), items 61063..61078.
    /// @param itemId Id of the item.
    /// @return True for a non-transferable item.
    bool isNonTransferable(int itemId);

}

#endif
