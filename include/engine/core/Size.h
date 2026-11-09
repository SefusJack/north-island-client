#ifndef NORTH_ISLAND_CORE_SIZE_H
#define NORTH_ISLAND_CORE_SIZE_H

namespace core {

    /// @brief A width and height (in pixels, or in cells where a caller says so).
    struct Size {
        int width = 0;
        int height = 0;
    };

}

#endif
