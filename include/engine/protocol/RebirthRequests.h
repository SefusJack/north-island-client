#ifndef NORTH_ISLAND_PROTOCOL_REBIRTH_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_REBIRTH_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 44/1: reborn a pet (RequestPetReborn).
    /// @param petPlace Pet place (u8, 1..4).
    /// @return The encoded payload.
    core::Bytes petRebornRequest(int petPlace);

}

#endif
