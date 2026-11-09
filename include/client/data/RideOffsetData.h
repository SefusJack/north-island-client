#ifndef NORTH_ISLAND_CLIENT_DATA_RIDE_OFFSET_DATA_H
#define NORTH_ISLAND_CLIENT_DATA_RIDE_OFFSET_DATA_H

#include "client/data/RideOffsetSet.h"

#include <map>

namespace client {

    /// @brief Rider offsets by ride skill (pet sprite id).
    /// @details Original: RiderOfsX / RiderOfsY of CH_RidePet.pas.
    /// @return Map from ride skill id to its offset set.
    const std::map<int, RideOffsetSet>& riderOffsetData();

    /// @brief The pet's own shift by ride skill (only skills 1185 and 1456).
    /// @details Original: RideOfsX / RideOfsY of CH_RidePet.pas.
    /// @return Map from ride skill id to its offset set.
    const std::map<int, RideOffsetSet>& rideOffsetData();

}

#endif
