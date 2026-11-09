#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_CLICK_SPEC_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_CLICK_SPEC_H

#include "client/services/input/ScriptObjectRef.h"
#include "ui/MouseButton.h"

namespace client {

    /// @brief A click on an object of the scene: which object, which button, how many clicks (2: a double click), Ctrl held.
    struct ScriptClickSpec {
        ScriptObjectRef target;
        ui::MouseButton button = ui::MouseButton::Left;
        int clicks = 1;
        bool ctrl = false;
    };

}

#endif
