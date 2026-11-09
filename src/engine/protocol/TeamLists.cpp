#include "protocol/TeamLists.h"

#include "core/ByteReader.h"

namespace protocol {

    std::vector<SceneTeam> decodeSceneTeams(const core::Bytes& payload) {
        std::vector<SceneTeam> teams;
        core::ByteReader reader(payload, 2);
        while (!reader.atEnd()) {
            SceneTeam team;
            team.leaderId = reader.i32();
            const int count = reader.u8();
            for (int i = 0; i < count; ++i) {
                team.memberIds.push_back(reader.i32());
            }
            if (reader.failed()) {
                break;
            }
            teams.push_back(std::move(team));
        }
        return teams;
    }

}
