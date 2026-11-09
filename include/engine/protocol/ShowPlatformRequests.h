#ifndef NORTH_ISLAND_PROTOCOL_SHOW_PLATFORM_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_SHOW_PLATFORM_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 62/55: the Show Platform was clicked.
    /// @details The server answers 62/54 with its restrained state. Layout: u16 piece.
    /// @param piece Furniture piece index of the platform.
    /// @return The encoded request payload.
    core::Bytes openShowPlatformRequest(int piece);

    /// @brief Builds request 62/49: put the vehicle of a Garage cell on the platform.
    /// @details Original: SetExhibitSlotClick. Layout: u16 piece, i32 cell.
    /// @param piece Furniture piece index of the platform.
    /// @param cell Garage cell holding the vehicle.
    /// @return The encoded request payload.
    core::Bytes showOnPlatformRequest(int piece, int cell);

    /// @brief Builds request 62/49: take the vehicle of a Garage cell off the platform.
    /// @details Original: ClearExhibitSlotClick. Layout: u16 cell, i32 0, i32 0.
    /// @param cell Garage cell whose vehicle is taken off.
    /// @return The encoded request payload.
    core::Bytes takeOffPlatformRequest(int cell);

}

#endif
