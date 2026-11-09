#ifndef NORTH_ISLAND_CLIENT_RULES_GM_IDS_H
#define NORTH_ISLAND_CLIENT_RULES_GM_IDS_H

#include <cstdint>

namespace client {

    /// @brief Tells whether an id is one the game keeps for GMs (100..400, of either server branch).
    /// @details Original: GetIdCategory 105..110.
    /// @param id Player id.
    /// @return True for a GM id.
    bool isGmId(std::int32_t id);

    /// @brief Tells whether an id is the game's own (1..30) or a GM's, kept off black lists.
    /// @details Original: GetIdCategory 2..4 or 105..110.
    /// @param id Player id.
    /// @return True for a staff id.
    bool isStaffId(std::int32_t id);

}

#endif
