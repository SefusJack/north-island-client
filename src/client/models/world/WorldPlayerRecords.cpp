#include "client/models/world/WorldPlayerRecords.h"

namespace client {

    std::optional<std::reference_wrapper<const protocol::OtherPlayerRecord>> setWorldPlayerMap(
        WorldPlayerRecords& records,
        std::int32_t playerId,
        int mapId,
        std::uint16_t x,
        std::uint16_t y
    ) {
        const auto found = records.find(playerId);
        if (found == records.end()) {
            return std::nullopt;
        }
        found->second.mapId = static_cast<std::uint16_t>(mapId);
        found->second.x = x;
        found->second.y = y;
        return std::cref(found->second);
    }

}
