#ifndef NORTH_ISLAND_PROTOCOL_PARKED_VEHICLE_H
#define NORTH_ISLAND_PROTOCOL_PARKED_VEHICLE_H

#include <cstdint>

namespace protocol {

    /// @brief A cell of the parking lot: the vehicle's item (0: empty cell), the cell, and the wear and fuel it keeps.
    /// @details Original: a record of server 23/153 and 23/154 (RecvParkList, RecvParkSlot).
    struct ParkedVehicle {
        int itemId = 0;
        int cell = 0;
        std::int32_t wear = 0;
        int fuel = 0;
    };

}

#endif
