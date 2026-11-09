#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_STALL_CASE_RECORD_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_STALL_CASE_RECORD_H

#include <cstdint>

namespace client {

    /// @brief One of the owner's four stall cases: furniture piece, place, item, count and price.
    /// @details Original: Re_PubConst.pas TStall4ItemRec (Player.FStallRecs); item 0 is a free case.
    struct StallCaseRecord {
        int furnIndex = 0;
        int posInFurn = 0;
        int itemId = 0;
        int count = 0;
        std::int32_t price = 0;
    };

}

#endif
