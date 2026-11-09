#ifndef NORTH_ISLAND_CLIENT_DATA_MAGIC_MAP_PLACES_H
#define NORTH_ISLAND_CLIENT_DATA_MAGIC_MAP_PLACES_H

#include "core/Point.h"

#include <string>

namespace client {

    /// @brief The forty places of the world map the teleport items open, numbered from 1.
    /// @details Original: MagicMapPlaceNames, MapPts_D_Storage and MagicMapPlaces.
    class MagicMapPlaces {
        public:
            static constexpr int kCount = 40; // places 1..40 of the teleport map

            /// @brief Gives the name of a place.
            /// @param place Place number (1..40).
            /// @return The name, or an empty string out of range.
            static const std::string& name(int place);

            /// @brief Gives where the place's button stands on the world map, in the map's own 12000-wide units.
            /// @param place Place number (1..40).
            /// @return The point, or (0, 0) out of range.
            static core::Point mapPoint(int place);

            /// @brief Tells whether the place's button is hidden.
            /// @param place Place number (1..40).
            /// @return True when hidden.
            static bool isHidden(int place);

            /// @brief Gives the cell the hint names.
            /// @param place Place number (1..40).
            /// @return The cell, or (0, 0) out of range.
            static core::Point cell(int place);

            /// @brief Gives the scene the place teleports to.
            /// @param place Place number (1..40).
            /// @return The map id, or 0 out of range.
            static int mapId(int place);
    };

}

#endif
