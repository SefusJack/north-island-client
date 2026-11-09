#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_GUILD_MEMBER_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_GUILD_MEMBER_H

#include "protocol/GuildRecord.h"

namespace client {

    /// @brief A member of the own guild: the server's record, the tick box state and the online state.
    /// @details Original: TOrgMemberRec; the online state comes from 39/61 (3 is offline until a server says otherwise).
    struct GuildMember {
        protocol::GuildMemberRecord record;
        bool checked = false;
        int onlineState = 3; // 3 = offline
    };

}

#endif
