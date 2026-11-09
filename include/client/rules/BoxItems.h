#ifndef NORTH_ISLAND_CLIENT_RULES_BOX_ITEMS_H
#define NORTH_ISLAND_CLIENT_RULES_BOX_ITEMS_H

namespace client {

    /// @brief Item ids that UseGridItem opens as boxes with a request of their own.
    /// @details Original: the Sweets Pack (23/74), the earlier boxes (23/75), the later boxes and lucky bags (23/128).
    class BoxItems {
        public:
            static constexpr int kSweetsPack = 34078; // item 34078: Sweets Pack
            static constexpr int kPurpleChest = 34268; // item 34268: Purple Chest
            static constexpr int kLuckyBagType = 41; // item type 41: lucky bags (e.g. 2011 Big Box, 2011 Ghost Pack)
            static constexpr int kMallGiftBoxType = 26; // item type 26: mall gift packs (e.g. 100,000 Gold Pack)
            static constexpr int kSuperHandbag = 0x853A; // item 34106 (0x853A): Super Handbag
            static constexpr int kCompanionVoucherSubType = 146; // item sub type 146: companion voucher (FItemSubType = 146 in AC_Item.pas)

            /// @brief Tells whether an item is on the 23/75 chain in UseGridItem.
            /// @param itemId Item id.
            /// @return True when it opens with the 23/75 request.
            static bool opensWithBox(int itemId);

            /// @brief Tells whether an item is on the 23/128 chain in UseGridItem (the lucky bags, item type 41, are tested by the caller).
            /// @param itemId Item id.
            /// @return True when it opens with the 23/128 request.
            static bool opensWithBox2(int itemId);
    };

}

#endif
