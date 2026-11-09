#ifndef NORTH_ISLAND_PROTOCOL_PET_TRADE_OFFER_H
#define NORTH_ISLAND_PROTOCOL_PET_TRADE_OFFER_H

#include "protocol/PetInfoBlock.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace protocol {

    /// @brief What the other side of a pet trade put up: gold and a pet (kind, level, HP, SP, name, amity, times upgraded, gear).
    /// @details Original: server 25/12 (RecvPartnerPetOffer).
    struct PetTradeOffer {
        std::int32_t gold = 0;
        int npcId = 0;
        int level = 0;
        std::int32_t maxHp = 0;
        std::int32_t hp = 0;
        int maxSp = 0;
        int sp = 0;
        std::string name;
        int amity = 0;
        int updateTimes = 0;
        std::vector<int> equipment;
        std::optional<PetInfoBlock> info;
    };

}

#endif
