#ifndef NORTH_ISLAND_CLIENT_SERVICES_SERVER_STATUS_CHANNEL_H
#define NORTH_ISLAND_CLIENT_SERVICES_SERVER_STATUS_CHANNEL_H

#include "core/Bytes.h"
#include "net/TcpSocket.h"

#include <cstdint>
#include <map>
#include <string>

namespace client {

    /// @brief Reader of the server status side channel (TCP 6416) that streams 3-byte records (u16 server id, u8 status 0..3).
    /// @details Original: the client sends nothing; no framing and no encryption.
    class ServerStatusChannel {
        public:
            static constexpr std::uint16_t kPort = 6416; // TCP port of the server status channel

            /// @brief Connects to the status port of a server and drops any buffered bytes.
            /// @param address Server address to connect to.
            void query(const std::string& address);

            /// @brief Closes the connection.
            void close() { m_socket.close(); }

            /// @brief Reads what arrived and replaces the known statuses with the latest records.
            /// @return True when statuses changed.
            bool update();

            /// @brief Status of one server (0 not connected, 1 all right, 2 busy, 3 crowded).
            /// @param serverId Server id from the record stream.
            /// @return The status, 0 when nothing is known about the server.
            int status(int serverId) const;

            /// @brief All servers named by the last read.
            /// @return Map of server id to status, as streamed.
            const std::map<int, int>& statuses() const { return m_statuses; }

        private:
            net::TcpSocket m_socket;
            core::Bytes m_buffer;
            std::map<int, int> m_statuses;
    };

}

#endif
