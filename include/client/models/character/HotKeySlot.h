#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_HOT_KEY_SLOT_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_HOT_KEY_SLOT_H

namespace client {

    /// @brief What one place of the F1-F8 bar holds: an item (kind 1) or a skill (kind 2) by id; kind 0 and id 0 when empty.
    /// @details Original: FHotKeySlots.
    struct HotKeySlot {
        int kind = 0;
        int id = 0;
    };

}

#endif
