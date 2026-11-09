#ifndef NORTH_ISLAND_PROTOCOL_DYE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_DYE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 23/38: the colour the dye of that bag place gives the part it changes.
    /// @param bagSlot Bag place (u8) of the dye.
    /// @param red Red level (0..9).
    /// @param green Green level (0..9).
    /// @param blue Blue level (0..9).
    /// @return The encoded request.
    core::Bytes dyeRequest(int bagSlot, int red, int green, int blue);

}

#endif
