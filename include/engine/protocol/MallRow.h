#ifndef NORTH_ISLAND_PROTOCOL_MALL_ROW_H
#define NORTH_ISLAND_PROTOCOL_MALL_ROW_H

namespace protocol {

    /// @brief One row of the item mall's catalog (75/1).
    /// @details Original: TShopItemRec. Discount 100 is none; mark 1 new, 2 hot, 3 timed, 4 special, 0 none; category 1 weapons, 2 armour,
    /// 3 / 4 goods, 5 furniture; the key is what the 2014 server wants back with a purchase.
    struct MallRow {
        int itemId = 0;
        int count = 0;
        int price = 0;
        int discount = 100; // 100 = no discount
        int hot = 0;
        int category = 0;
        int key = 0;
    };

}

#endif
