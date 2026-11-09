#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_SIGHTING_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_SIGHTING_H

#include "core/Point.h"

#include <optional>
#include <string>

namespace client {

    /// @brief An object of the scene as a script sees it: what it is, where on the map, and a screen point the scene's hit test accepts (none while off the screen or covered).
    struct ScriptSighting {
        std::string label;
        core::Point map;
        std::optional<core::Point> screen;
    };

}

#endif
