#ifndef NORTH_ISLAND_PROTOCOL_TEAM_LISTS_H
#define NORTH_ISLAND_PROTOCOL_TEAM_LISTS_H

#include "core/Bytes.h"
#include "protocol/SceneTeam.h"

#include <vector>

namespace protocol {

    /// @brief Decodes server 13/6, the teams on the map: records of i32 leader, u8 n, n x i32 member.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The complete team records read before any truncation.
    std::vector<SceneTeam> decodeSceneTeams(const core::Bytes& payload);

}

#endif
