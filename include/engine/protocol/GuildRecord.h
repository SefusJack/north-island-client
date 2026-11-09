#ifndef NORTH_ISLAND_PROTOCOL_GUILD_RECORD_H
#define NORTH_ISLAND_PROTOCOL_GUILD_RECORD_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace protocol {

    /// @brief A guild member in server 39/2 (original RecvGuildInfo).
    /// @details Layout: i32 id, pstr8 name, u8 level, job, reborn, element, body type, head, i32 colour code 1, i32 colour code 5, pstr8 nickname, i32 contribution, 5 x u8 rights (1 set authority, 2 invite, 3-5 not used), 5 x u8 not read.
    struct GuildMemberRecord {
        std::int32_t id = 0;
        std::string name;
        int level = 0;
        int job = 0;
        int reborn = 0;
        int element = 0;
        int bodyType = 0;
        int head = 0;
        std::int32_t colorCode1 = 0;
        std::int32_t colorCode5 = 0;
        std::string nickname;
        std::int32_t contribution = 0;
        std::array<bool, 5> rights{};
    };

    /// @brief The whole guild (server 39/2): the lord first, then the vice lords, then the members.
    struct GuildInfo {
        std::string name;
        int viceCount = 0;
        std::vector<GuildMemberRecord> members;
        std::string rules;
        double disbandDate = 0;
        double founded = 0;
    };

}

#endif
