#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_VEHICLE_GAUGE_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_VEHICLE_GAUGE_H

#include <string>

namespace client {

    /// @brief What the status panel shows of the vehicle ridden: the item's icon in the size of its cells, its rails and how much of it and its fuel is left.
    /// @details Original: DrawMainStatus, FObjects 139. A capsule's icon is shown as it is.
    struct VehicleGauge {
        std::string icon;
        int cellWidth = 1;
        int cellHeight = 1;
        bool capsule = false;
        double durability = 1.0;
        double fuel = 0.0;
        bool hasFuel = false;
    };

}

#endif
