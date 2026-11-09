#include "net/TcpSocket.h"

#include "Platform.h"
#include "core/Log.h"

#include <cstring>
#include <utility>

namespace net {

    namespace {

        /// @brief Converts the stored handle back to the native socket type.
        /// @param handle Handle as kept in the socket object.
        /// @return The native socket.
        SOCKET native(std::uintptr_t handle) { return static_cast<SOCKET>(handle); }

        /// @brief Puts a socket into non-blocking mode.
        /// @param s The native socket.
        /// @return True when the mode was set.
        bool setNonBlocking(SOCKET s) {
#ifdef _WIN32
            u_long enabled = 1;
            return ioctlsocket(s, FIONBIO, &enabled) == 0;
#else
            const int flags = fcntl(s, F_GETFL, 0);
            return flags >= 0 && fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
        }

    }

    TcpSocket::~TcpSocket() { close(); }

    TcpSocket::TcpSocket(
        TcpSocket&& other
    ) noexcept
        : m_handle(std::exchange(other.m_handle, kInvalid)),
          m_state(std::exchange(other.m_state, State::Closed)),
          m_peer(std::move(other.m_peer)) {}

    TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept {
        if (this != &other) {
            close();
            m_handle = std::exchange(other.m_handle, kInvalid);
            m_state = std::exchange(other.m_state, State::Closed);
            m_peer = std::move(other.m_peer);
        }
        return *this;
    }

    bool TcpSocket::connect(const std::string& host, std::uint16_t port) {
        close();
        m_reset = false;
        m_peer = host + ':' + std::to_string(port);
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        addrinfo* found = nullptr;
        const std::string service = std::to_string(port);
        const int resolved = getaddrinfo(host.c_str(), service.c_str(), &hints, &found);
        if (resolved != 0 || found == nullptr) {
            core::logWarning("cannot resolve ", host, " (error ", resolved, ')');
            m_state = State::Failed;
            return false;
        }
        const SOCKET s = ::socket(found->ai_family, found->ai_socktype, found->ai_protocol);
        if (s == INVALID_SOCKET || !setNonBlocking(s)) {
            core::logWarning("cannot open a socket for ", m_peer, " (error ", lastSocketError(), ')');
            freeaddrinfo(found);
            if (s != INVALID_SOCKET) {
                closeSocket(s);
            }
            m_state = State::Failed;
            return false;
        }
        const int result = ::connect(s, found->ai_addr, static_cast<SocketLength>(found->ai_addrlen));
        freeaddrinfo(found);
        m_handle = static_cast<std::uintptr_t>(s);
        const int error = result == 0 ? 0 : lastSocketError();
        if (result == 0) {
            m_state = State::Connected;
        } else if (isWouldBlock(error)) {
            m_state = State::Connecting;
        } else {
            core::logWarning("cannot connect to ", m_peer, " (error ", error, ')');
            close();
            m_state = State::Failed;
            return false;
        }
        return true;
    }

    void TcpSocket::close() {
        if (m_handle != kInvalid) {
            closeSocket(native(m_handle));
            m_handle = kInvalid;
        }
        m_state = State::Closed;
    }

    void TcpSocket::abandon() {
        if (m_state != State::Connecting) {
            return;
        }
        core::logWarning("no answer from ", m_peer, ", gave up");
        close();
        m_state = State::Failed;
    }

    void TcpSocket::poll() {
        if (m_state != State::Connecting) {
            return;
        }
        const SOCKET s = native(m_handle);
        fd_set writable;
        fd_set failed;
        FD_ZERO(&writable);
        FD_ZERO(&failed);
        FD_SET(s, &writable);
        FD_SET(s, &failed);
        timeval noWait{0, 0};
        const int ready = ::select(static_cast<int>(s + 1), nullptr, &writable, &failed, &noWait);
        if (ready <= 0) {
            return;
        }
        int error = 0;
        SocketLength length = sizeof(error);
        getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &length);
        if (FD_ISSET(s, &failed) || error != 0) {
            core::logWarning("cannot connect to ", m_peer, " (error ", error, ')');
            closeSocket(s);
            m_handle = kInvalid;
            m_state = State::Failed;
            return;
        }
        m_state = State::Connected;
    }

    int TcpSocket::send(const core::Bytes& data) {
        if (m_state != State::Connected || data.empty()) {
            return 0;
        }
        const int sent = ::send(native(m_handle), reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0);
        if (sent < 0) {
            if (isWouldBlock(lastSocketError())) {
                return 0;
            }
            m_reset = true;
            close();
            return -1;
        }
        return sent;
    }

    core::Bytes TcpSocket::receive() {
        core::Bytes received;
        if (m_state != State::Connected) {
            return received;
        }
        char buffer[8192]; // receive chunk size in bytes
        for (;;) {
            const int count = ::recv(native(m_handle), buffer, sizeof(buffer), 0);
            if (count > 0) {
                received.insert(received.end(), buffer, buffer + count);
                continue;
            }
            if (count == 0 || !isWouldBlock(lastSocketError())) {
                m_reset = count < 0;
                close();
            }
            break;
        }
        return received;
    }

}
