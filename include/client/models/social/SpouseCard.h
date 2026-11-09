#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_SPOUSE_CARD_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_SPOUSE_CARD_H

#include "client/models/character/CharacterLook.h"

#include <array>
#include <cstdint>
#include <string>

namespace client {

    /// @brief What the social window's Marry tab shows of the spouse: who, looks, attributes, where and whether online.
    /// @details Original: packet 45/3, MarryCard.FObjects.V[1].
    struct SpouseCard {
        std::int32_t id = 0;
        std::string name;
        std::string guild;
        std::string nickname;
        CharacterLook look;
        std::array<std::uint16_t, 6> equipment{};
        int element = 0;
        int level = 0;
        int job = 0;
        int strength = 0;
        int constitution = 0;
        int intelligence = 0;
        int wisdom = 0;
        int agility = 0;
        std::string location;
        int onlineState = 3; // online state 3: offline
    };

}

#endif
