#ifndef NORTH_ISLAND_UI_LIST_ITEM_H
#define NORTH_ISLAND_UI_LIST_ITEM_H

#include "core/Color.h"

#include <optional>
#include <string>

namespace ui {

    /// @brief One row of a ListBox, with an optional colour.
    struct ListItem {
        std::string text;
        std::optional<core::Color> color;
    };

}

#endif
