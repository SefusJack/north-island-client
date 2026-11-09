#ifndef NORTH_ISLAND_PROTOCOL_PLAYER_APPEARANCE_H
#define NORTH_ISLAND_PROTOCOL_PLAYER_APPEARANCE_H

#include "core/ByteReader.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief How a player looks in the player records (server 3 and 4): body type, head, body sprite, the two colour codes and worn items.
    struct PlayerAppearance {
        int bodyType = 1;
        int head = 0;
        int bodySprite = 0;
        std::int32_t colorCode1 = 0;
        std::int32_t colorCode2 = 0;
        std::vector<std::uint16_t> equipment;

        /// @brief Reads the worn items (u8 n, n x u16 item id).
        /// @param reader Reader positioned at the list.
        void readEquipment(core::ByteReader& reader);
    };

}

#endif
