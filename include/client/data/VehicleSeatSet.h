#ifndef NORTH_ISLAND_CLIENT_DATA_VEHICLE_SEAT_SET_H
#define NORTH_ISLAND_CLIENT_DATA_VEHICLE_SEAT_SET_H

#include "client/data/RideOffsetSet.h"

namespace client {

    /// @brief Where the people sit on one vehicle kind: the driver's offsets and those of passenger seats 2..4.
    /// @details Original: CH_Traffic.pas. Each set is by body type (0..4) and direction (1..8 of the riding pose).
    struct VehicleSeatSet {
        RideOffsetSet driver;
        RideOffsetSet passengers[3];
    };

}

#endif
