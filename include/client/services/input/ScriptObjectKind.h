#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_OBJECT_KIND_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_OBJECT_KIND_H

namespace client {

    /// @brief What a test script can name in the scene (clickobj, where ...).
    enum class ScriptObjectKind {
        Self,
        Npc,
        NpcIndex,
        Player,
        Ground,
        Furniture,
        FurnitureItem,
    };

}

#endif
