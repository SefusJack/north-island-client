#include "client/rules/TargetNames.h"

#include <algorithm>

namespace client {

    std::vector<std::string> targetNames(
        const FriendList& friends,
        const Guild& guild,
        std::int32_t ownId,
        bool withFriends,
        bool withGuild
    ) {
        std::vector<std::string> names;
        const auto listed = [&names](const std::string& name) { return !name.empty() && std::find(names.begin(), names.end(), name) != names.end(); };
        if (withFriends) {
            for (int index = 1; index <= friends.count(); ++index) {
                const std::string& name = friends.at(index).record.name;
                if (!listed(name)) {
                    names.push_back(name);
                }
            }
        }
        if (withGuild) {
            for (int index = 0; index < guild.count(); ++index) {
                const protocol::GuildMemberRecord& member = guild.at(index).record;
                if (!listed(member.name) && member.id != ownId && member.id != 0) {
                    names.push_back(member.name);
                }
            }
        }
        return names;
    }

}
