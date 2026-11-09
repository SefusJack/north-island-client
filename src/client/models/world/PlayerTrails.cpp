#include "client/models/world/PlayerTrails.h"

namespace client {

    void PlayerTrails::at(std::int32_t playerId, core::Point position, bool placed) {
        Player& player = m_players[playerId];
        player.position = position;
        if (placed) {
            player.trail.reset(position);
        } else {
            player.trail.update(position);
        }
    }

    void PlayerTrails::scatter(std::int32_t playerId, core::Point position, std::mt19937& random) {
        Player& player = m_players[playerId];
        player.position = position;
        player.trail.scatter(position, random);
    }

    core::Point PlayerTrails::position(std::int32_t playerId) const {
        const auto found = m_players.find(playerId);
        return found != m_players.end() ? found->second.position : core::Point{};
    }

    core::Point PlayerTrails::point(std::int32_t playerId, std::size_t index) const {
        const auto found = m_players.find(playerId);
        return found != m_players.end() ? found->second.trail.at(index) : core::Point{};
    }

}
