#include "client/rules/GmIds.h"

namespace client {

    namespace {

        constexpr std::int32_t kBranchOffset = 4500000; // id offset added to players of the second server branch

        /// @brief Strips the second server branch's offset from an id.
        /// @details Original: StripServerOffset.
        /// @param id Player id.
        /// @return The id without the branch offset.
        std::int32_t stripBranch(std::int32_t id) { return id > kBranchOffset && id <= 2 * kBranchOffset ? id - kBranchOffset : id; }

    }

    bool isGmId(std::int32_t id) {
        id = stripBranch(id);
        return id >= 100 && id <= 400; // GM id range
    }

    bool isStaffId(std::int32_t id) {
        const std::int32_t stripped = stripBranch(id);
        return (stripped >= 1 && stripped <= 30) || isGmId(id); // the game's own ids 1..30
    }

}
