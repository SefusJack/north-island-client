#ifndef NORTH_ISLAND_FORMATS_SERVER_ENTRY_H
#define NORTH_ISLAND_FORMATS_SERVER_ENTRY_H

#include "core/Color.h"

#include <optional>
#include <string>

namespace formats {

    /// @brief One game server of SERVER.INI ("Name*a.b.c.d").
    /// @details id is group code * 100 + position in the group (from 1), how the status channel names it. color comes from a <R>, <G> or <B> tag
    /// in the name. port is the game port (0: the game's, 6414); SERVER.INI has none, a server added by address may.
    struct ServerEntry {
        std::string name;
        std::string address;
        int id = 0;
        std::optional<core::Color> color;
        int port = 0;
    };

}

#endif
