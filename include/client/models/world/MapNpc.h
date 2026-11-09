#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_MAP_NPC_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_MAP_NPC_H

#include "client/models/world/MapCharacter.h"
#include "client/models/world/NpcRoutine.h"
#include "formats/NpcInfo.h"

#include <cstdint>
#include <map>
#include <vector>

namespace client {

    /// @brief An NPC of the map: its place in the map's list, what NPC it is, whether it shows and how it stands and walks.
    /// @details Original: eve.Emg entry; index is the server's 22/x index, info comes from Npc.dat, the server can hide it.
    /// Position, direction and walking are as for players; its name is the NPC's.
    struct MapNpc {
        int index = 0;
        formats::NpcInfo info;
        bool visible = true;
        // 255: no held frame, the scene object animates
        int holdFrame = 255;
        // 255: no held frame, the action plays
        int actionHoldFrame = 255;
        int mapSign = 0;
        int eventCount = 0;
        core::Point triggerHalfSize;
        std::map<int, std::vector<core::Point>> eventWalks;
        double ownPace = 0.0;
        bool startsNear = false;
        bool held = false;
        std::uint64_t nearEndedMs = 0;
        bool eventWalking = false;
        bool hideOnArrival = false;
        int showState = 1; // FNpcShowState 1: shown
        std::uint64_t showEndMs = 0;
        core::Point home;
        int homeDirection = 0;
        int speed = 2; // default speed number
        int spriteOffsetY = 0;
        int drawOffsetY = 0;
        MapCharacter body;
        NpcRoutine routine;

        /// @brief Gives the y the NPC is sorted by in the draw order.
        /// @details Original: BuildDrawOrder; a type 10 scene object is sorted on its own (DrawSceneObjects), the others by y plus the role's draw offset.
        /// @return The sort y.
        int drawOrderY() const { return info.type == 10 ? body.position.y : body.position.y + drawOffsetY; } // NPC type 10: scene object sorted on its own

        /// @brief Tells whether the NPC shows a name (types 6, 8 and 9, things to pick up and the like, show none).
        /// @return True when the name shows.
        bool showsName() const { return info.type != 6 && info.type != 8 && info.type != 9; } // NPC types 6, 8, 9: pick-up things without a name

        /// @brief Tells whether the NPC is a thing rather than a person.
        /// @details Original: IsSceneObjectNpc (types 6, 9 and 10).
        /// @return True for a scene object.
        bool isSceneObject() const { return info.type == 6 || info.type == 9 || info.type == 10; } // NPC types 6, 9, 10: scene objects
    };

}

#endif
