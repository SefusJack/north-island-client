#ifndef NORTH_ISLAND_CLIENT_RULES_RADIO_H
#define NORTH_ISLAND_CLIENT_RULES_RADIO_H

namespace client {

    /// @brief The kind of a radio set that opens the world channel (worn in the sixth equipment slot, or kept in the own house).
    /// @details Original: TFItem.GetRadioKind. 1 the house radio 39070, 2 item 34076, 3 item 34132 (whose lines show as "(All)").
    /// @param itemId The item id.
    /// @return The radio kind, 0 for not a radio.
    int radioKind(int itemId);

}

#endif
