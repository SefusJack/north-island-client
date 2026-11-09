#include "net/NetworkSystem.h"

#include "Platform.h"

namespace net {

    NetworkSystem::NetworkSystem() {
#ifdef _WIN32
        WSADATA data;
        m_ready = WSAStartup(MAKEWORD(2, 2), &data) == 0; // Winsock version 2.2
#else
        m_ready = true;
#endif
    }

    NetworkSystem::~NetworkSystem() {
#ifdef _WIN32
        if (m_ready) {
            WSACleanup();
        }
#endif
    }

}
