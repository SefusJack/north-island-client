#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_CASE_SALE_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_CASE_SALE_H

#include <cstdint>

namespace protocol {

    /// @brief So many of a stall case were sold (server 62/58 option 2): the owner, the case's number, how many and the piece.
    /// @details Original: RecvStallCaseUpdate.
    struct FurnitureCaseSale {
        std::int32_t ownerId = 0;
        int caseNo = 0;
        int count = 0;
        int furnIndex = 0;
    };

}

#endif
