#ifndef NORTH_ISLAND_PROTOCOL_SERVER_SELECTED_H
#define NORTH_ISLAND_PROTOCOL_SERVER_SELECTED_H

#include "core/Bytes.h"

#include <cstdint>
#include <optional>
#include <string>

namespace protocol {

    /// @brief Server message 1/9: the server accepted the connection and the client shows the login form.
    /// @details Layout: 01 09, u16 server id, u8 grouped flag, server name (rest of the payload).
    struct ServerSelected {
        std::uint16_t serverId = 0;
        bool grouped = false;
        std::string serverName;

        /// @brief Decodes the server-selected message.
        /// @param payload Packet payload including the action and sub id bytes.
        /// @return The decoded message, or nullopt when the ids differ or the payload is truncated.
        static std::optional<ServerSelected> decode(const core::Bytes& payload);
    };

}

#endif
