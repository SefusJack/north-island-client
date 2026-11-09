#ifndef NORTH_ISLAND_PROTOCOL_TENT_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_TENT_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds request 65/1: the tent is entered (i32 owner).
    /// @details Original: RequestUseTent (SendServerMsg 65/x in DZ_Connect.pas).
    /// @param ownerId Player id of the tent owner.
    /// @return The encoded request payload.
    core::Bytes enterTentRequest(std::int32_t ownerId);

    /// @brief Builds request 65/2: the tent is taken back (u8 bag place).
    /// @param bagSlot Bag place of the tent.
    /// @return The encoded request payload.
    core::Bytes takeBackTentRequest(int bagSlot);

    /// @brief Builds request 23/94: the Tent Palette or the Shape Clay is used (u8 bag place, u8 set).
    /// @details Original: RequestChangeCamp.
    /// @param bagSlot Bag place of the item.
    /// @param set 1 for the Tent Palette, 2 for the Shape Clay.
    /// @return The encoded request payload.
    core::Bytes tentPaletteRequest(int bagSlot, int set);

    /// @brief Builds request 65/12: the style chosen (u8 style, u8 bag place, u8 set).
    /// @details Original: ConfirmChangeCamp.
    /// @param style Chosen style index.
    /// @param bagSlot Bag place of the item used.
    /// @param set Item set, as in tentPaletteRequest.
    /// @return The encoded request payload.
    core::Bytes tentStyleRequest(int style, int bagSlot, int set);

}

#endif
