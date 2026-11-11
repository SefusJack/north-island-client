#ifndef NORTH_ISLAND_FORMATS_EVENT_SCENE_H
#define NORTH_ISLAND_FORMATS_EVENT_SCENE_H

#include "formats/SceneDoor.h"
#include "formats/SceneNpc.h"
#include "formats/SceneSurface.h"

#include <map>
#include <set>
#include <vector>

namespace formats {

    /// @brief What eve.Emg places on one map: the NPCs, the doors, the events that start when the player comes near their NPC and what the events show.
    /// @details Original: near events have flag 2 (TEvent.FFlag1); the surfaces are by index; the mines, goods, the rest of the events, groups and warps between are passed over. battleMusic maps the scripted battles by their number (what 11/250 names) to the music each plays (TBattle.FValue291: 1 BGM0014, 2 and 3 BGM0003, else the map's usual).
    struct EventScene {
        std::vector<SceneNpc> npcs;
        std::vector<SceneDoor> doors;
        std::set<int> nearEvents;
        std::map<int, SceneSurface> surfaces;
        std::map<int, int> battleMusic;
    };

}

#endif
