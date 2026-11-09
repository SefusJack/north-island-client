#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_EMOTE_SLOT_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_EMOTE_SLOT_H

namespace client {

    /// @brief One Alt+1..8 slot of the emote tab: empty, an emotion (E101..E124 by number 1..24) or an action (btn_Action_1..24).
    /// @details Original: FHotkeySlotKinds, FEmotionNo.
    struct EmoteSlot {
        /// @brief What a slot holds; the values are the file's kind codes.
        enum class Kind { Empty = 0, Emotion = 1, Action = 2 };

        Kind kind = Kind::Empty;
        int number = 0;
    };

}

#endif
