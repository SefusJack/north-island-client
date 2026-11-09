#ifndef NORTH_ISLAND_CLIENT_MODELS_ITEMS_GIFT_CARD_H
#define NORTH_ISLAND_CLIENT_MODELS_ITEMS_GIFT_CARD_H

#include <vector>

namespace client {

    /// @brief One Gift Exchange Card's list of what it can be exchanged for: the card item, the list version and the items to choose from.
    /// @details Original: GiftItemData of HL_Menu.pas.
    struct GiftCard {
        /// @brief One item a card can be exchanged for.
        struct Choice {
            int itemId = 0;
            int count = 0;
        };

        int id = 0;
        int version = 0;
        std::vector<Choice> choices;
    };

}

#endif
