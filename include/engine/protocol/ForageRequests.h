#ifndef NORTH_ISLAND_PROTOCOL_FORAGE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_FORAGE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 23/88: the pet goes to find that item.
    /// @param boneSlot Bag place (u8) of the Dog Bone.
    /// @param petSlot Pet place (u8).
    /// @param itemId Item to find (u16).
    /// @param targets How many items the form holds (u8).
    /// @param ordinal Which one this is (u8).
    /// @return The encoded request.
    core::Bytes forageStartRequest(
        int boneSlot,
        int petSlot,
        int itemId,
        int targets,
        int ordinal
    );

    /// @brief Builds client 23/91: stop the foraging.
    /// @return The encoded request.
    core::Bytes forageCancelRequest();

}

#endif
