#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_MARK_SLOT_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_MARK_SLOT_H

namespace client {

    /// @brief One of the player's 200 quest mark places: a counter mark and how far it got.
    /// @details Original: FMarkSlots.
    struct MarkSlot {
        int markId = 0;
        int count = 0;
    };

}

#endif
