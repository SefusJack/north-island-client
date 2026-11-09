#ifndef NORTH_ISLAND_NET_GAME_CONNECTION_H
#define NORTH_ISLAND_NET_GAME_CONNECTION_H

#include "core/Bytes.h"
#include "net/TcpSocket.h"
#include "protocol/FrameDecoder.h"

#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace net {

    /// @brief The connection to the game server (TCP 6414): frames and encrypts outgoing messages, decodes incoming ones.
    /// @details Like the original client it writes at most one queued frame per 100 ms. Call update() once per frame.
    class GameConnection {
        public:
            static constexpr std::uint16_t kGamePort = 6414; // TCP port of the game server
            static constexpr std::uint64_t kSendIntervalMs = 100; // minimum ms between two sent frames
            static constexpr std::uint64_t kConnectTimeoutMs = 15000; // connect gives up after 15 s

            /// @brief What happened to the connection.
            /// @details Reset follows the Disconnected of a connection that ended with an error (the original's OnError after OnDisconnect).
            enum class Event { Connected, ConnectFailed, Disconnected, Reset };

            /// @brief Starts connecting to the game server, closing any earlier connection.
            /// @param address Host name or dotted address.
            /// @param port TCP port.
            void connect(const std::string& address, std::uint16_t port = kGamePort);

            /// @brief Closes the connection and drops the queued messages.
            void close();

            /// @brief Queues a message payload (action byte first); it is framed and encrypted here.
            /// @param payload The message payload.
            void send(const core::Bytes& payload);

            /// @brief Polls the socket, receives, writes at most one queued frame per send interval and raises events.
            /// @param nowMs The current time in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Adds a message as if the server had sent it (the test script's `recv`), read before the real ones.
            /// @param payload The message payload.
            void inject(const core::Bytes& payload) { m_injected.push_back(payload); }

            /// @brief Takes the next received message.
            /// @return The payload, or nothing when none is complete.
            std::optional<core::Bytes> nextMessage() {
                if (!m_injected.empty()) {
                    core::Bytes payload = std::move(m_injected.front());
                    m_injected.pop_front();
                    return payload;
                }
                return m_decoder.next();
            }

            /// @brief Takes the next connection event.
            /// @return The event, or nothing when none is waiting.
            std::optional<Event> nextEvent();

            /// @brief Whether the socket is connecting or connected.
            /// @return True when open.
            bool isOpen() const;

            /// @brief Whether messages are queued and not yet written (a close drops them).
            /// @return True when sends are pending.
            bool sendsPending() const { return !m_sendQueue.empty(); }

        private:
            TcpSocket m_socket;
            protocol::FrameDecoder m_decoder;
            std::deque<core::Bytes> m_injected;
            std::deque<core::Bytes> m_sendQueue;
            std::deque<Event> m_events;
            std::uint64_t m_lastSendMs = 0;
            TcpSocket::State m_lastState = TcpSocket::State::Closed;
            std::optional<std::uint64_t> m_connectStartMs;
            bool m_heardFrom = false;
    };

}

#endif
