#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_PENDING_CLICK_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_PENDING_CLICK_H

#include "client/services/input/ScriptClickSpec.h"
#include "core/Point.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief A click on a scene object that is waiting a moment: the mouse is on the object's point and the click comes after a frame or two (the hover of the house pieces is worked out when the world is drawn).
    struct ScriptPendingClick {
        ScriptClickSpec spec;
        std::string what;
        core::Point at;
        std::uint64_t dueMs = 0;
        int moves = 0;
    };

}

#endif
