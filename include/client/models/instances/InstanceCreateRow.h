#ifndef NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_CREATE_ROW_H
#define NORTH_ISLAND_CLIENT_MODELS_INSTANCES_INSTANCE_CREATE_ROW_H

#include <string>

namespace client {

    /// @brief An instance the create form offers: scene, title and display texts.
    /// @details Original: AddCreateDupRow.
    struct InstanceCreateRow {
        int sceneId = 0;
        std::string title;
        std::string level;
        std::string time;
        std::string players;
        std::string guildOnly;
        std::string description;
    };

}

#endif
