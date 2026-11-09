#ifndef NORTH_ISLAND_PROTOCOL_GUILD_LISTS_H
#define NORTH_ISLAND_PROTOCOL_GUILD_LISTS_H

#include "core/Bytes.h"
#include "protocol/GuildRecord.h"

#include <optional>

namespace protocol {

    /// @brief Decodes server 39/2: guild name, vice lords, slots (1 lord, 2..5 vice lords, 6.. members), rules, disband and founding dates, allies.
    /// @details Original: RecvGuildInfo.
    /// @param payload Packet payload.
    /// @return The guild info, or nothing when malformed.
    std::optional<GuildInfo> decodeGuildInfo(const core::Bytes& payload);

}

#endif
