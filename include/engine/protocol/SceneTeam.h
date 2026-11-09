#ifndef NORTH_ISLAND_PROTOCOL_SCENE_TEAM_H
#define NORTH_ISLAND_PROTOCOL_SCENE_TEAM_H

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief A team on the map: its leader and members (at most three).
    /// @details Original: server 13/6, RecvSceneTeams.
    struct SceneTeam {
        std::int32_t leaderId = 0;
        std::vector<std::int32_t> memberIds;
    };

}

#endif
