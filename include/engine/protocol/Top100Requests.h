#ifndef NORTH_ISLAND_PROTOCOL_TOP100_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_TOP100_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 84/1: a list of the Top 100 window (u8 kind, u8 page).
    /// @param kind 0 the sizes, 1 level, 2 money, 3 Q&A, 4 PK, 5 and 6 the event lists.
    /// @param page Page of the list.
    /// @return The encoded request payload.
    core::Bytes top100Request(int kind, int page);

}

#endif
