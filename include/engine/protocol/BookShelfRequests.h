#ifndef NORTH_ISLAND_PROTOCOL_BOOK_SHELF_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_BOOK_SHELF_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 5/9 (Bookcase) or 5/11 (Desk): put a blueprint into the shelf.
    /// @param desk True for the Desk, false for the Bookcase.
    /// @param bagSlot Bag place (u8) of the blueprint.
    /// @param place Shelf place (1..10) it goes into.
    /// @return The encoded request.
    core::Bytes putInBookShelfRequest(bool desk, int bagSlot, int place);

    /// @brief Builds client 5/10 (Bookcase) or 5/12 (Desk): take a blueprint back to the bag.
    /// @param desk True for the Desk, false for the Bookcase.
    /// @param place Shelf place (u8).
    /// @return The encoded request.
    core::Bytes takeFromBookShelfRequest(bool desk, int place);

}

#endif
