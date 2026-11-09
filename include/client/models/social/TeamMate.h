#ifndef NORTH_ISLAND_CLIENT_MODELS_SOCIAL_TEAM_MATE_H
#define NORTH_ISLAND_CLIENT_MODELS_SOCIAL_TEAM_MATE_H

#include "client/models/character/CharacterLook.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief What the group window shows of a teammate: who, looks, level, job, element, HP and SP (known only from the server's 8/3).
    /// @details Original: TWorldPlayer; the instance window shows the nickname too.
    struct TeamMate {
        std::string name;
        std::string nickname;
        CharacterLook look;
        CharacterLook sceneLook;
        int level = 0;
        int reborn = 0;
        int job = 0;
        int element = 0;
        std::int32_t hp = 0;
        int sp = 0;
        int maxHp = 0;
        int maxSp = 0;
    };

}

#endif
