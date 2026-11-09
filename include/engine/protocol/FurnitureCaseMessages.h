#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_CASE_MESSAGES_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_CASE_MESSAGES_H

#include "core/Bytes.h"
#include "protocol/FurnitureCaseSale.h"
#include "protocol/FurnitureCaseUpdate.h"

#include <cstdint>
#include <optional>

namespace protocol {

    /// @brief Decodes server 62/58 option 1: i32 owner, u8 case, the record (u8 piece, u8 place, u16 item, u8 count, i32 price).
    /// @param payload Packet payload.
    /// @return The update, or nothing when malformed.
    std::optional<FurnitureCaseUpdate> decodeFurnitureCaseUpdate(const core::Bytes& payload);

    /// @brief Decodes server 62/58 option 2: i32 owner, u16 case, u8 count, u16 piece.
    /// @param payload Packet payload.
    /// @return The sale, or nothing when malformed.
    std::optional<FurnitureCaseSale> decodeFurnitureCaseSale(const core::Bytes& payload);

    /// @brief Builds client 62/58 option 1: a new case with the bag stack.
    /// @param piece Furniture piece (u8).
    /// @param bagSlot Bag place (u8).
    /// @param price Price (i32).
    /// @return The encoded request.
    core::Bytes openCaseRequest(int piece, int bagSlot, std::int32_t price);

    /// @brief Builds client 62/58 option 2: add a bag stack to a case.
    /// @param piece Furniture piece (u8).
    /// @param bagSlot Bag place (u8).
    /// @param caseNo Case number (u8).
    /// @param room Room left in the case (u8) when the stack would not all fit; 0 means not sent.
    /// @return The encoded request.
    core::Bytes addToCaseRequest(int piece, int bagSlot, int caseNo, int room);

    /// @brief Builds client 62/58 option 3: take that many back from a case.
    /// @param piece Furniture piece (u8).
    /// @param caseNo Case number (u8).
    /// @param count How many (u8).
    /// @return The encoded request.
    core::Bytes takeFromCaseRequest(int piece, int caseNo, int count);

    /// @brief Builds client 62/58 option 4: change a case's price.
    /// @param piece Furniture piece (u8).
    /// @param caseNo Case number (u8).
    /// @param price New price (i32).
    /// @return The encoded request.
    core::Bytes repriceCaseRequest(int piece, int caseNo, std::int32_t price);

    /// @brief Builds client 62/59: buy from a case.
    /// @param ownerId Owner's player id (i32).
    /// @param caseNo Case number (u16).
    /// @param count How many (u8).
    /// @param price The case's price (i32).
    /// @return The encoded request.
    core::Bytes buyFromCaseRequest(std::int32_t ownerId, int caseNo, int count, std::int32_t price);

}

#endif
