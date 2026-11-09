#ifndef NORTH_ISLAND_PROTOCOL_STALL_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_STALL_REQUESTS_H

#include "core/Bytes.h"
#include "protocol/OwnStall.h"

#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

    /// @brief Builds request 23/30: open the own item stall.
    /// @details Layout: pstr8 name, u8 sign, then u8 bag slot and i32 price per item.
    /// @param name Stall name.
    /// @param sign Index of the stall picture.
    /// @param prices Bag slot and price of each offered item.
    /// @return The encoded request payload.
    core::Bytes openItemStallRequest(const std::string& name, int sign, const std::vector<OwnStall::Price>& prices);

    /// @brief Builds request 23/31: pull the own stall.
    /// @return The encoded request payload.
    core::Bytes closeItemStallRequest();

    /// @brief Builds request 23/32: look at someone's item stall (i32 owner).
    /// @param ownerId Player id of the stall owner.
    /// @return The encoded request payload.
    core::Bytes browseItemStallRequest(std::int32_t ownerId);

    /// @brief Builds request 23/60: open the own pet stall.
    /// @details Layout: pstr8 name, u8 pet count, then u8 pet place and i32 price per pet.
    /// @param name Stall name.
    /// @param prices Pet place and price of each offered pet.
    /// @return The encoded request payload.
    core::Bytes openPetStallRequest(const std::string& name, const std::vector<OwnStall::Price>& prices);

    /// @brief Builds request 23/61: pull the own pet stall.
    /// @return The encoded request payload.
    core::Bytes closePetStallRequest();

    /// @brief Builds request 23/62: look at someone's pet stall (i32 owner).
    /// @param ownerId Player id of the stall owner.
    /// @return The encoded request payload.
    core::Bytes browsePetStallRequest(std::int32_t ownerId);

    /// @brief Builds request 23/63: buy pets from a pet stall (i32 owner, then the owner's pet places).
    /// @param ownerId Player id of the stall owner.
    /// @param places Pet places bought.
    /// @return The encoded request payload.
    core::Bytes buyStallPetsRequest(std::int32_t ownerId, const std::vector<int>& places);

    /// @brief One line of a purchase from a stall: the owner's bag place, how many, the item and the key the stall gave it.
    struct StallPurchase {
        int stallSlot = 0;
        int count = 0;
        int itemId = 0;
        std::uint32_t echoKey = 0;
    };

    /// @brief Builds request 23/33: buy from an item stall.
    /// @details Layout: i32 owner, then u8 slot, u8 count, u16 item, u32 key per line.
    /// @param ownerId Player id of the stall owner.
    /// @param lines The purchase lines.
    /// @return The encoded request payload.
    core::Bytes buyFromStallRequest(std::int32_t ownerId, const std::vector<StallPurchase>& lines);

}

#endif
