#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_STALL_CASE_CHANGE_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_STALL_CASE_CHANGE_H

#include "client/models/house/StallCaseRecord.h"

namespace client {

    /// @brief What a stall case message changed: case number, own flag, record before and after.
    /// @details Original: the part of RecvStallCaseUpdate the forms act on.
    struct StallCaseChange {
        int caseNo = 0;
        bool own = false;
        StallCaseRecord before;
        StallCaseRecord after;
    };

}

#endif
