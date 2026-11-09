#ifndef NORTH_ISLAND_PROTOCOL_PLAYER_ID_H
#define NORTH_ISLAND_PROTOCOL_PLAYER_ID_H

#include <cstdint>

namespace protocol {

    /// @brief Removes the second server branch's offset from a player id (StripServerOffset).
    /// @details What the account's login id (63/2) is compared with.
    /// @param playerId The player id.
    /// @return The id without the offset.
    // ids 4500001..9000000 belong to the second server branch, which adds 4500000 to the real player id
    inline std::int32_t stripServerOffset(std::int32_t playerId) { return playerId > 4500000 && playerId <= 9000000 ? playerId - 4500000 : playerId; }

}

#endif
