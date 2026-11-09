#ifndef NORTH_ISLAND_FORMATS_SERVER_GROUP_H
#define NORTH_ISLAND_FORMATS_SERVER_GROUP_H

#include "core/Color.h"
#include "formats/ServerEntry.h"

#include <optional>
#include <string>
#include <vector>

namespace formats {

    /// @brief A server group of SERVER.INI ("NN[Group name]<C>code" and the server lines below it).
    struct ServerGroup {
        int number = 0;
        std::string name;
        int code = 0;
        std::optional<core::Color> color;
        std::vector<ServerEntry> servers;
    };

}

#endif
