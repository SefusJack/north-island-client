#ifndef NORTH_ISLAND_PROTOCOL_REMOTE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_REMOTE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds 23/76: the Remote Control was used (its form opens).
    /// @param bagSlot Bag place (u8).
    /// @return The encoded payload.
    core::Bytes remoteOpenRequest(int bagSlot);

    /// @brief Builds 23/78: the Remote Control's settings (TTH_AutoPlayForm.StartAutoPlay).
    /// @param ticks The ticks (u16, bit n - 1 for tick n).
    /// @param hpPercent HP percent (sent as i32 HP% * 100000 + pet HP%).
    /// @param spPercent SP percent (sent as i32 SP% * 100000 + pet SP%).
    /// @param petHpPercent Pet HP percent.
    /// @param petSpPercent Pet SP percent.
    /// @param deathLimit Deaths that make the player leave (u16).
    /// @param petDeathLimit Deaths of the pet that make the player leave (u16).
    /// @param leaveMinutes Minutes to leave in (u16).
    /// @param itemFrom First place of the bag the supply takes from (u8).
    /// @param itemTo Last place of the bag the supply takes from (u8).
    /// @return The encoded payload.
    core::Bytes remoteStartRequest(
        int ticks,
        int hpPercent,
        int spPercent,
        int petHpPercent,
        int petSpPercent,
        int deathLimit,
        int petDeathLimit,
        int leaveMinutes,
        int itemFrom,
        int itemTo
    );

    /// @brief Builds 23/79: the Remote Control was stopped by the player.
    /// @return The encoded payload.
    core::Bytes remoteStopRequest();

    /// @brief Builds 23/81: the Remote Control gave up, no supply was left.
    /// @return The encoded payload.
    core::Bytes remoteGaveUpRequest();

    /// @brief Builds 5/8: the player is being taken out of the game (SendAfkAutoStop).
    /// @return The encoded payload.
    core::Bytes remoteLeaveRequest();

}

#endif
