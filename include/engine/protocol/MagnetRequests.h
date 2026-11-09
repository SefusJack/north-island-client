#ifndef NORTH_ISLAND_PROTOCOL_MAGNET_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_MAGNET_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds 23/90 1: the wearer asks a player to be teleported to him.
    /// @param bagSlot Bag place of the Magic Magnet (u8).
    /// @param playerId Id of the asked player (i32).
    /// @return The encoded payload.
    core::Bytes askMagnetRequest(int bagSlot, std::int32_t playerId);

    /// @brief Builds 23/90 2: the asked player agrees.
    /// @return The encoded payload.
    core::Bytes agreeMagnetRequest();

    /// @brief Builds 23/90 3: the asked player refuses.
    /// @return The encoded payload.
    core::Bytes refuseMagnetRequest();

    /// @brief Builds 23/90 4: the asked player cannot be teleported.
    /// @details Reason codes follow RecvMagnetResult: 7 in a battle, 9 scene, 12 stall, 13 vehicle, 15 / 16 level, 17 gone, 18 no answer.
    /// @param reason Reason code (u8).
    /// @return The encoded payload.
    core::Bytes cannotMagnetRequest(int reason);

}

#endif
