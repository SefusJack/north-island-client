#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_STALL_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_STALL_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds client 62/21: the items of the bag place onto a shelf at that price.
    /// @param piece Furniture piece (u16).
    /// @param itemId Item (u16).
    /// @param bagSlot Bag place (u8).
    /// @param count How many (u8).
    /// @param price Price (i32).
    /// @return The encoded request.
    core::Bytes putOnShelfRequest(
        int piece,
        int itemId,
        int bagSlot,
        int count,
        std::int32_t price
    );

    /// @brief Builds client 62/24: the same as the shelf, into a vending machine.
    /// @param piece Furniture piece (u16).
    /// @param itemId Item (u16).
    /// @param bagSlot Bag place (u8).
    /// @param count How many (u8).
    /// @param price Price (i32).
    /// @param row Row (u8); 0 is the first free one.
    /// @return The encoded request.
    core::Bytes putInVendingRequest(
        int piece,
        int itemId,
        int bagSlot,
        int count,
        std::int32_t price,
        int row
    );

    /// @brief Builds client 62/22: the owner takes an item back from a shelf.
    /// @param stallSlot Stall number (u16).
    /// @return The encoded request.
    core::Bytes takeBackFromShelfRequest(int stallSlot);

    /// @brief Builds client 62/25: the owner takes an item back from a vending machine.
    /// @param stallSlot Stall number (u16).
    /// @return The encoded request.
    core::Bytes takeBackFromVendingRequest(int stallSlot);

    /// @brief Builds client 62/23: buy from a shelf.
    /// @param ownerId Owner's player id (i32).
    /// @param stallSlot Stall number (u16).
    /// @param count How many (u8).
    /// @param echoItem The u16 the item's record gave.
    /// @param echoPrice The i32 the item's record gave.
    /// @return The encoded request.
    core::Bytes buyFromShelfRequest(
        std::int32_t ownerId,
        int stallSlot,
        int count,
        int echoItem,
        std::int32_t echoPrice
    );

    /// @brief Builds client 62/26: buy from a vending machine.
    /// @param ownerId Owner's player id (i32).
    /// @param stallSlot Stall number (u16).
    /// @param count How many (u8).
    /// @param echoItem The u16 the item's record gave.
    /// @param echoPrice The i32 the item's record gave.
    /// @return The encoded request.
    core::Bytes buyFromVendingRequest(
        std::int32_t ownerId,
        int stallSlot,
        int count,
        int echoItem,
        std::int32_t echoPrice
    );

}

#endif
