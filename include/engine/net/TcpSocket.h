#ifndef NORTH_ISLAND_NET_TCP_SOCKET_H
#define NORTH_ISLAND_NET_TCP_SOCKET_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace net {

    /// @brief Non-blocking IPv4 TCP client socket, polled by its owner.
    class TcpSocket {
        public:
            /// @brief Connection state of the socket.
            enum class State { Closed, Connecting, Connected, Failed };

            TcpSocket() = default;

            /// @brief Closes the connection if still open.
            ~TcpSocket();

            /// @brief Takes over the connection of another socket.
            /// @param other Socket to move from; left closed.
            TcpSocket(
                TcpSocket&& other
            ) noexcept;

            /// @brief Closes this connection and takes over the other socket's.
            /// @param other Socket to move from; left closed.
            /// @return This socket.
            TcpSocket& operator=(TcpSocket&& other) noexcept;
            TcpSocket(const TcpSocket&) = delete;
            TcpSocket& operator=(const TcpSocket&) = delete;

            /// @brief Starts a non-blocking connect; poll() reports when the connection is up.
            /// @param host Host name or IPv4 address.
            /// @param port TCP port.
            /// @return True when the attempt was started.
            bool connect(const std::string& host, std::uint16_t port);

            /// @brief Closes the connection.
            void close();

            /// @brief Gives up a connection still Connecting (no answer in time); the state becomes Failed.
            void abandon();

            /// @brief Advances Connecting to Connected or Failed.
            void poll();

            /// @brief Sends bytes without blocking.
            /// @param data Bytes to send.
            /// @return Bytes written (possibly fewer than given), or -1 when the connection broke.
            int send(const core::Bytes& data);

            /// @brief Reads everything that has arrived; sets the state to Closed when the peer closed the connection.
            /// @return The received bytes.
            core::Bytes receive();

            /// @brief Current connection state.
            /// @return The state.
            State state() const { return m_state; }

            /// @brief Tells whether the connection ended with an error (a reset), not with the peer's orderly close.
            /// @return True after a reset.
            bool wasReset() const { return m_reset; }

        private:
            using Handle = std::uintptr_t;
            static constexpr Handle kInvalid = ~static_cast<Handle>(0);

            Handle m_handle = kInvalid;
            State m_state = State::Closed;
            bool m_reset = false;
            std::string m_peer;
    };

}

#endif
