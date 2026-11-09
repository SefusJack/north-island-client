#ifndef NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_INFO_H
#define NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_INFO_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief What the windows show of one instance: map, creator, player count, rule texts and remaining time.
    /// @details Original: rDupDetail.
    struct InstanceInfo {
        std::int32_t creatorId = 0;
        int mapId = 0;
        int members = 0;
        std::string title;
        std::string description;
        std::string minLevel;
        std::string maxPlayers;
        std::string timeLimit;
        std::string guildOnly;
        int remainingMinutes = 0;
        std::string remainPrefix;
    };

}

#endif
