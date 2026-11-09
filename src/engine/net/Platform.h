#ifndef NORTH_ISLAND_NET_PLATFORM_H
#define NORTH_ISLAND_NET_PLATFORM_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketLength = int;

/// @brief The last socket error of this thread.
/// @return The platform's error code (WSAGetLastError).
inline int lastSocketError() { return WSAGetLastError(); }

/// @brief Tells whether an error only means the operation is not finished yet.
/// @param error A platform socket error code.
/// @return True for would-block / in-progress.
inline bool isWouldBlock(int error) { return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS; }

/// @brief Closes a socket.
/// @param s The socket.
inline void closeSocket(SOCKET s) { closesocket(s); }
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
using SOCKET = int;
using SocketLength = socklen_t;
constexpr SOCKET INVALID_SOCKET = -1;

/// @brief The last socket error of this thread.
/// @return The platform's error code (errno).
inline int lastSocketError() { return errno; }

/// @brief Tells whether an error only means the operation is not finished yet.
/// @param error A platform socket error code.
/// @return True for would-block / again / in-progress.
inline bool isWouldBlock(int error) { return error == EWOULDBLOCK || error == EAGAIN || error == EINPROGRESS; }

/// @brief Closes a socket.
/// @param s The socket.
inline void closeSocket(SOCKET s) { ::close(s); }
#endif

#endif
