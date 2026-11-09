#ifndef NORTH_ISLAND_PROTOCOL_EGG_MACHINE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_EGG_MACHINE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds client 62/31: put the stack of a bag place in the Fortune Egg machine.
    /// @param piece Furniture piece (u16).
    /// @param bagSlot Bag place (u8).
    /// @param place Place in the machine (u8).
    /// @return The encoded request.
    core::Bytes putEggStackRequest(int piece, int bagSlot, int place);

    /// @brief Builds client 62/32: what a play costs.
    /// @param piece Furniture piece (u16).
    /// @param price Price (i32); 0 stops selling.
    /// @return The encoded request.
    core::Bytes setEggPriceRequest(int piece, std::int32_t price);

    /// @brief Builds client 62/33: play the machine at its price.
    /// @param ownerId Player id of the house owner (i32).
    /// @param piece Furniture piece (u16).
    /// @param price Price (i32).
    /// @return The encoded request.
    core::Bytes playEggMachineRequest(std::int32_t ownerId, int piece, std::int32_t price);

    /// @brief Builds client 62/34: a Token Egg goes into the house's Fortune Egg machine.
    /// @details Original: UseGridItem, item kind 1 of the item's slot 9.
    /// @param bagSlot Bag place (u8) of the egg.
    /// @return The encoded request.
    core::Bytes feedEggMachineRequest(int bagSlot);

    /// @brief Builds client 62/35: take the stack of that place back.
    /// @param piece Furniture piece (u16).
    /// @param place Place in the machine (u8).
    /// @return The encoded request.
    core::Bytes retakeEggStackRequest(int piece, int place);

    /// @brief Builds client 62/36: ask for the machine's price and eggs.
    /// @param piece Furniture piece (u16).
    /// @param ownerId Player id of the house owner (i32).
    /// @return The encoded request.
    core::Bytes askEggMachineRequest(int piece, std::int32_t ownerId);

}

#endif
