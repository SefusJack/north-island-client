#ifndef NORTH_ISLAND_CLIENT_RULES_VEHICLE_SEATS_H
#define NORTH_ISLAND_CLIENT_RULES_VEHICLE_SEATS_H

#include "core/Point.h"

namespace client {

    /// @brief Where someone sits on a vehicle.
    /// @details Original: CalcVehicleSeatPos, GetDriverSeatOffset, GetPassengerSeatOffset.
    class VehicleSeats {
        public:
            /// @brief Gives the shift from the vehicle's feet to the rider's of a seat.
            /// @param bodyType Body type of the rider.
            /// @param slot Pose slot the rider is in (18..25, or 46..53).
            /// @param seat Seat number (1 the driver, 2..4 the passengers).
            /// @param vehicleKind Kind of the vehicle.
            /// @return The shift; nothing (zero) for a slot outside the riding poses.
            static core::Point offset(int bodyType, int slot, int seat, int vehicleKind);

            /// @brief Tells who is drawn in a step of a vehicle's riders (Original: GetVehicleSeatDrawOrder); the one seen from behind first.
            /// @param vehicleKind Kind of the vehicle.
            /// @param slot Pose slot the driver is in (18..25, 46..53).
            /// @param step Draw step, 1..seat count.
            /// @return The driver (0) or the passenger of that number (1..3).
            static int drawOrder(int vehicleKind, int slot, int step);
    };

}

#endif
