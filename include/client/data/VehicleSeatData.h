#ifndef NORTH_ISLAND_CLIENT_DATA_VEHICLE_SEAT_DATA_H
#define NORTH_ISLAND_CLIENT_DATA_VEHICLE_SEAT_DATA_H

#include "client/data/VehicleSeatSet.h"

#include <map>

namespace client {

    /// @brief Seat offsets by vehicle kind (1..17).
    /// @details Original: DriverSeatOfsX/Y, PassengerSeatOfsX/Y and SeatOfsX/Y of CH_Traffic.pas.
    /// @return Map from vehicle kind to its seat set.
    const std::map<int, VehicleSeatSet>& vehicleSeatData();

}

#endif
