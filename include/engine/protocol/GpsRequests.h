#ifndef NORTH_ISLAND_PROTOCOL_GPS_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_GPS_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 74/1: record the place the player stands on as that point.
    /// @param gpsSlot Bag place (u8) of the GPS.
    /// @param point Point (1..3).
    /// @return The encoded request.
    core::Bytes gpsMarkRequest(int gpsSlot, int point);

    /// @brief Builds client 74/2: go to the point (sent once the departure's light has played).
    /// @param gpsSlot Bag place (u8) of the GPS.
    /// @param point Point (u8).
    /// @return The encoded request.
    core::Bytes gpsTeleportRequest(int gpsSlot, int point);

    /// @brief Builds client 74/4: raise the GPS one step.
    /// @param crystalSlot Bag place (u8) of the Memory Crystal.
    /// @param gpsSlot Bag place (u8) of the GPS.
    /// @return The encoded request.
    core::Bytes gpsUpgradeRequest(int crystalSlot, int gpsSlot);

    /// @brief Builds client 74/3 with option 1: a click on the Dreaming Pill icon asks how long it lasts.
    /// @details Original: SendServerMsg(74, 3, 1).
    /// @return The encoded request.
    core::Bytes dreamingPillRequest();

}

#endif
