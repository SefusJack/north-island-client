#include "net/GameConnection.h"

#include "core/Log.h"
#include "protocol/Framing.h"

namespace net {

    void GameConnection::connect(const std::string& address, std::uint16_t port) {
        close();
        core::logInfo("connecting to ", address, ':', port);
        m_socket.connect(address, port);
        m_lastState = TcpSocket::State::Closed;
        m_connectStartMs.reset();
        m_heardFrom = false;
    }

    void GameConnection::close() {
        m_socket.close();
        m_sendQueue.clear();
        m_decoder = protocol::FrameDecoder();
        m_lastState = TcpSocket::State::Closed;
    }

    bool GameConnection::isOpen() const { return m_socket.state() == TcpSocket::State::Connecting || m_socket.state() == TcpSocket::State::Connected; }

    void GameConnection::send(const core::Bytes& payload) {
        core::Bytes wire = protocol::frame(payload);
        protocol::applyStreamKey(wire);
        m_sendQueue.push_back(std::move(wire));
    }

    std::optional<GameConnection::Event> GameConnection::nextEvent() {
        if (m_events.empty()) {
            return std::nullopt;
        }
        const Event event = m_events.front();
        m_events.pop_front();
        return event;
    }

    void GameConnection::update(std::uint64_t nowMs) {
        m_socket.poll();
        if (m_socket.state() == TcpSocket::State::Connecting) {
            if (!m_connectStartMs) {
                m_connectStartMs = nowMs;
            } else if (nowMs - *m_connectStartMs >= kConnectTimeoutMs) {
                m_socket.abandon();
            }
        }

        if (m_socket.state() == TcpSocket::State::Connected) {
            const core::Bytes received = m_socket.receive();
            if (!received.empty()) {
                if (!m_heardFrom) {
                    core::logInfo("the server answered (", received.size(), " bytes)");
                    m_heardFrom = true;
                }
                m_decoder.feed(received);
            }
        }

        if (m_socket.state() == TcpSocket::State::Connected && !m_sendQueue.empty() && nowMs - m_lastSendMs >= kSendIntervalMs) {
            core::Bytes& head = m_sendQueue.front();
            const int sent = m_socket.send(head);
            if (sent == static_cast<int>(head.size())) {
                m_sendQueue.pop_front();
            } else if (sent > 0) {
                head.erase(head.begin(), head.begin() + sent);
            }
            m_lastSendMs = nowMs;
        }

        const TcpSocket::State state = m_socket.state();
        if (state != m_lastState) {
            if (state == TcpSocket::State::Connected) {
                core::logInfo("connection up after ", m_connectStartMs ? nowMs - *m_connectStartMs : 0, " ms");
                m_events.push_back(Event::Connected);
            } else if (state == TcpSocket::State::Failed) {
                m_events.push_back(Event::ConnectFailed);
            } else if (state == TcpSocket::State::Closed && m_lastState == TcpSocket::State::Connected) {
                core::logInfo("server closed the connection");
                m_events.push_back(Event::Disconnected);
                if (m_socket.wasReset()) {
                    m_events.push_back(Event::Reset);
                }
                m_sendQueue.clear();
            }
            m_lastState = state;
        }
    }

}
