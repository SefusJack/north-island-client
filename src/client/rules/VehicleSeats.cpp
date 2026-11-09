#include "client/rules/VehicleSeats.h"

#include "client/data/VehicleSeatData.h"

namespace client {

    core::Point VehicleSeats::offset(int bodyType, int slot, int seat, int vehicleKind) {
        // vehicle kinds 1, 13, 14 use the riding poses starting at slot 18 (action group 18); other kinds start at slot 46
        const bool lowPose = vehicleKind == 1 || vehicleKind == 13 || vehicleKind == 14;
        // row = slot - 17 (poses from 18) or slot - 45 (poses from 46)
        const int row = slot - (lowPose ? 17 : 45);
        const auto found = vehicleSeatData().find(vehicleKind);
        // row 8 and body type 4 are the table's last indices; seats 1..4
        if (row < 0 || row > 8 || bodyType < 0 || bodyType > 4 || found == vehicleSeatData().end() || seat < 1 || seat > 4) {
            return {};
        }
        const RideOffsetSet& set = seat == 1 ? found->second.driver : found->second.passengers[seat - 2];
        return {set.x[bodyType][row], set.y[bodyType][row]};
    }

    int VehicleSeats::drawOrder(int vehicleKind, int slot, int step) {
        // draw order of kind 11: row per pose slot 46..53, column per step 1..4
        constexpr int kOrderOf11[8][4] = {{0, 1, 2, 3}, {1, 3, 0, 2}, {1, 3, 0, 2}, {3, 1, 2, 0}, {3, 2, 1, 0}, {2, 3, 0, 1}, {2, 0, 3, 1}, {0, 2, 1, 3}};
        if (step < 1 || step > 4) { // steps 1..4 (at most 4 riders)
            return 0;
        }
        switch (vehicleKind) {
            case 1: // kind 1: driver and one passenger
                if (step > 2) {
                    return 0;
                }
                return (slot >= 21 && slot <= 23) == (step == 1) ? 1 : 0; // passenger first for the poses 21..23
            case 4:
            case 7:
            case 8:
            case 9: // kinds 4, 7, 8, 9: driver and one passenger; passenger first for the poses 49..51
                if (step > 2) {
                    return 0;
                }
                return (slot >= 49 && slot <= 51) == (step == 1) ? 1 : 0;
            case 3: // kind 3: driver and one passenger; passenger first for the poses 46..50
                if (step > 2) {
                    return 0;
                }
                return (slot >= 46 && slot <= 50) == (step == 1) ? 1 : 0;
            case 11: // kind 11: four riders, order from the table by pose slot 46..53
                return slot >= 46 && slot <= 53 ? kOrderOf11[slot - 46][step - 1] : 0;
            default: // other kinds: the driver only
                return 0;
        }
    }

}
