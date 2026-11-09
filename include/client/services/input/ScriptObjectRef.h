#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_OBJECT_REF_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_OBJECT_REF_H

#include "client/services/input/ScriptObjectKind.h"

namespace client {

    /// @brief One object a script names: its kind and its number.
    struct ScriptObjectRef {
        ScriptObjectKind kind = ScriptObjectKind::Self;
        int id = 0;
    };

}

#endif
