#include "protocol/RebornWishRequests.h"

namespace protocol {

    core::Bytes rebornWishRequest(int npcId) {
        constexpr std::uint8_t kRebornAction = 66; // action 66: reborn
        constexpr std::uint8_t kWishSub = 11; // 66/11: make a reborn wish
        // 0xFF masks one byte, 8 shifts to the high byte: the u16 npc id, low byte first
        return {kRebornAction, kWishSub, static_cast<std::uint8_t>(npcId & 0xFF), static_cast<std::uint8_t>(npcId >> 8 & 0xFF)};
    }

}
