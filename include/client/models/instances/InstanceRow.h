#ifndef NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_ROW_H
#define NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_ROW_H

#include <string>

namespace client {

    /// @brief A row of the list of open instances: map, title, texts, creator and message.
    /// @details Original: AddDupOrderRow.
    struct InstanceRow {
        int mapId = 0;
        std::string title;
        std::string level;
        std::string players;
        std::string guildLimit;
        std::string creator;
        std::string message;
    };

}

#endif
