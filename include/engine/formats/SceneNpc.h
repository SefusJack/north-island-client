#ifndef NORTH_ISLAND_FORMATS_SCENE_NPC_H
#define NORTH_ISLAND_FORMATS_SCENE_NPC_H

#include "core/Point.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace formats {

    /// @brief An NPC placed on a map by eve.Emg.
    /// @details Its place in the map's list (the index the server's 22/x messages use), which NPC kind (Npc.dat), where it stands (map pixels,
    /// its feet), which way it faces (1 north, then clockwise: 2 north-east ... 8 north-west), how it moves (1 stands, 2 walks a path, 3 wanders,
    /// 4 / 5 other walks), what a path walk does at the end (1 walks it again, 2 back the other way, else stops), its path (points and a value
    /// per point: wait times) and speed. Flag bit 0: shown from the start. Its events (eve.Emg event ids; a click on an NPC without any does nothing)
    /// and the half sizes of the area that sets them off when the player walks in (FValue688 / FValue692; 0: none, as for every NPC of move kinds 1 to 3).
    /// The walks its events may send it on (TNpcItem: up to ten paths of map points, by number). drawOffsetY is TRole.FDrawOffsetY (the record's
    /// last field), added to the NPC's y in the order things are drawn in. mapSign is the minimap sign it starts with on every map load
    /// (Icon_Sign_<n>: 5 blue circle, 6 "!", 7 "?"; 0 none).
    struct SceneNpc {
        /// @brief One point of an NPC's path with its wait value.
        struct PathPoint {
            core::Point position;
            std::int32_t value = 0;
        };

        int index = 0;
        std::string name;
        int flags = 0;
        core::Point position;
        int npcId = 0;
        int direction = 1;
        int moveKind = 1;
        int pathEnd = 0;
        std::vector<PathPoint> path;
        int speed = 2;
        std::vector<int> eventIds;
        core::Point triggerHalfSize;
        std::map<int, std::vector<core::Point>> eventWalks;
        int drawOffsetY = 0;
        int mapSign = 0;
    };

}

#endif
