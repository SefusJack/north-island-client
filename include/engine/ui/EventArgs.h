#ifndef NORTH_ISLAND_UI_EVENT_ARGS_H
#define NORTH_ISLAND_UI_EVENT_ARGS_H

#include "core/Point.h"

#include <string>

namespace ui {

    /// @brief What a handler learns about an event: source element, list index, mouse position and drop target.
    struct EventArgs {
        std::string source;
        int index = -1;
        core::Point position;
        bool ctrl = false;
        std::string target;
        int targetIndex = -1;
        core::Point grab;
    };

}

#endif
