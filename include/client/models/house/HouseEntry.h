#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_ENTRY_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_ENTRY_H

#include "core/Point.h"

#include <functional>

namespace client {

    /// @brief A person or pet moving in a house floor room: its feet cell and how to draw it.
    /// @details Original: the room's painter draws it in the turn of its cell, after the furniture on the cell's first level (DrawSceneObjects' house branch).
    struct HouseEntry {
        core::Point cell;
        std::function<void()> draw;
    };

}

#endif
