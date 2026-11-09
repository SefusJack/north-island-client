#ifndef NORTH_ISLAND_FORMATS_SCENE_DOOR_H
#define NORTH_ISLAND_FORMATS_SCENE_DOOR_H

#include "core/Point.h"
#include "core/Rect.h"

#include <string>
#include <vector>

namespace formats {

    /// @brief A door of a map in eve.Emg.
    /// @details The cells that trigger it (stepping in asks the server, client 20/8 with the index), the events it runs, and the light picture
    /// it shows (cursorKind 1 DoorLight, 2 SmallDoorLight) standing on a map point. mapSign is the minimap sign it starts with on every map load
    /// (Icon_Sign_<n>; 0 none), until a server 22/12.
    struct SceneDoor {
        int index = 0;
        std::string name;
        int flags = 0;
        core::Rect area;
        std::vector<int> eventIds;
        int kind = 0;
        core::Point lightPosition;
        int mapSign = 0;

        /// @brief The cursor kind of the door (what DrawEventLights and the minimap arrows test).
        /// @details Original: ConvertSceneDoors; file kind 1 is cursor kind 0, 2 is 2, 3 is 1.
        /// @return The cursor kind.
        int cursorKind() const { return kind == 3 ? 1 : kind == 2 ? 2 : 0; }
    };

}

#endif
