#ifndef NORTH_ISLAND_CLIENT_DATA_RIDE_OFFSET_SET_H
#define NORTH_ISLAND_CLIENT_DATA_RIDE_OFFSET_SET_H

namespace client {

    /// @brief Pixel offsets of a rider on one kind of pet, by body type (0..4) and direction (1..8 of the riding pose).
    struct RideOffsetSet {
        int x[5][9];
        int y[5][9];
    };

}

#endif
