#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCREEN_POINT_FINDER_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCREEN_POINT_FINDER_H

#include "core/Point.h"
#include "core/Rect.h"

#include <functional>
#include <optional>

namespace client {

    /// @brief Finds a point to click for a test script: one a hit test accepts, so the game's own test (an opaque pixel of the sprite, the topmost element) takes it for the object.
    struct ScreenPointFinder {
        using Hits = std::function<bool(core::Point)>;

        /// @brief Finds the point of an area that the hit test accepts, nearest the middle of the area.
        /// @details A point whose neighbours (3 pixels around, then 1) hit as well is preferred, so a click a pixel off still lands.
        /// @param area The area to search.
        /// @param screen The visible screen rectangle the area is cut to.
        /// @param hits Hit test, true when a point takes the object.
        /// @return The point, empty when nothing in the area hits.
        static std::optional<core::Point> find(const core::Rect& area, const core::Rect& screen, const Hits& hits);
    };

}

#endif
