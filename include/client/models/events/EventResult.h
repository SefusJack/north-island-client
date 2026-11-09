#ifndef NORTH_ISLAND_CLIENT_MODELS_EVENTS_EVENT_RESULT_H
#define NORTH_ISLAND_CLIENT_MODELS_EVENTS_EVENT_RESULT_H

#include <cstdint>

namespace client {

    /// @brief One result of a map event the server runs.
    /// @details Original: server 20/1..6, TEventManage.FCurResult. The kind is what happens (0 a notice or NPC control, 1 a talk line, 2 a warp, 3 a fight, 4 an NPC walk, 5 a movie, 6 a dialog or store, 11 an NPC turns, 14 the player turns, 15 music); then its mode, target, option, value and data id (a Talk.Dat line, a surface, a fight, ...).
    struct EventResult {
        int kind = 0;
        int mode = 0;
        int target = 0;
        int option = 0;
        std::int32_t value = 0;
        int dataId = 0;
    };

}

#endif
