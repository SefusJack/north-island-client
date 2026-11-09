#ifndef NORTH_ISLAND_PROTOCOL_REFER_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_REFER_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 23/103 (u8 kind 1: with a glass; the form never asks with points): search an item for its materials.
    /// @param itemId Item id (u16).
    /// @param itemSlot Bag place of the item (u8).
    /// @param glassSlot Bag place of the glass (u8).
    /// @return The encoded payload.
    core::Bytes referSearchRequest(int itemId, int itemSlot, int glassSlot);

}

#endif
