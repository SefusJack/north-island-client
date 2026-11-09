#ifndef NORTH_ISLAND_PROTOCOL_MAID_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_MAID_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 76/sub: a request from the Wit Maid's menu.
    /// @details Subs: 1 the item bank, 2 the inn's storage, 3 the clothes, 250 the menu is done with (a form it opened closed).
    /// @param sub Sub code.
    /// @param furnIndex The maid's furniture number (u8).
    /// @param costume Costume bytes (u8 menu choice, u8 bag place of the suit) when there is one.
    /// @return The encoded payload.
    core::Bytes maidRequest(int sub, int furnIndex, const core::Bytes& costume = {});

}

#endif
