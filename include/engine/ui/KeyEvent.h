#ifndef NORTH_ISLAND_UI_KEY_EVENT_H
#define NORTH_ISLAND_UI_KEY_EVENT_H

#include "ui/Key.h"

namespace ui {

    /// @brief A key press or release with the state of the modifier keys.
    struct KeyEvent {
        Key key = Key::Other;
        bool ctrl = false;
        bool shift = false;
        bool alt = false;
    };

}

#endif
