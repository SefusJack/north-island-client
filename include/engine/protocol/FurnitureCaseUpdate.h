#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_CASE_UPDATE_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_CASE_UPDATE_H

#include <cstdint>

namespace protocol {

    /// @brief A stall case of a display case changed (server 62/58 option 1): whose it is, its number (1..4) and the record (piece, place, item, count, price).
    /// @details Original: RecvStallCaseUpdate, 9-byte record TStall4ItemRec. An item of 0 or a count of 0 is a case that is gone.
    struct FurnitureCaseUpdate {
        std::int32_t ownerId = 0;
        int caseNo = 0;
        int furnIndex = 0;
        int posInFurn = 0;
        int itemId = 0;
        int count = 0;
        std::int32_t price = 0;
    };

}

#endif
