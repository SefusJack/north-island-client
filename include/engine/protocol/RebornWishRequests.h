#ifndef NORTH_ISLAND_PROTOCOL_REBORN_WISH_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_REBORN_WISH_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 66/11: wish for a pet kind at the Wishing God (the stars are counted by the client, taken by the server).
    /// @param npcId The pet's kind (u16).
    /// @return The encoded payload.
    core::Bytes rebornWishRequest(int npcId);

}

#endif
