#ifndef NORTH_ISLAND_UI_LAYOUT_SOURCE_H
#define NORTH_ISLAND_UI_LAYOUT_SOURCE_H

#include <string>

namespace ui {

    /// @brief The text of one layout and the name (its file name) errors are reported with.
    struct LayoutSource {
        std::string name;
        std::string text;
    };

}

#endif
