#ifndef NORTH_ISLAND_PROTOCOL_UTENSIL_STALL_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_UTENSIL_STALL_REQUESTS_H

#include "core/Bytes.h"
#include "protocol/OwnStall.h"
#include "protocol/UtensilStallGoods.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace protocol {

    /// @brief Builds request 23/65: open the own large-item stall.
    /// @details Original: the form's FStallRequest. Layout: pstr8 name, u8 how many, then u8 bag place and i32 price each.
    /// @param name Stall name.
    /// @param prices Bag place and price of each offered item.
    /// @return The encoded request payload.
    core::Bytes openUtensilStallRequest(const std::string& name, const std::vector<OwnStall::Price>& prices);

    /// @brief Builds request 23/66: pull the own large-item stall.
    /// @return The encoded request payload.
    core::Bytes closeUtensilStallRequest();

    /// @brief Builds request 23/67: look at someone's large-item stall (i32 owner).
    /// @param ownerId Player id of the stall owner.
    /// @return The encoded request payload.
    core::Bytes browseUtensilStallRequest(std::int32_t ownerId);

    /// @brief One line of a purchase: the owner's list place of the piece and how many (always the piece's count).
    struct UtensilPurchase {
        int listPlace = 0;
        int count = 0;
    };

    /// @brief Builds request 23/68: buy from a large-item stall.
    /// @details Original: BuySelectedUtensils builds the whole message. Layout: i32 owner, then u8 list place and u8 count per line.
    /// @param ownerId Player id of the stall owner.
    /// @param lines The purchase lines.
    /// @return The encoded request payload.
    core::Bytes buyUtensilsRequest(std::int32_t ownerId, const std::vector<UtensilPurchase>& lines);

    /// @brief Decodes server 23/133, the pieces of a large-item stall.
    /// @details Original: RecvUtensilStallList. Layout: i32 owner, 12 bytes the client skips, then 15-byte pieces (u8 place, u16 item, u8 count, u8 wear, i32 price, i32 used, u16 extra). The game counts the pieces as (length - 17) div 14 and keeps the first three.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The goods, or nullopt when the header is truncated.
    std::optional<UtensilStallGoods> decodeUtensilStallGoods(const core::Bytes& payload);

}

#endif
