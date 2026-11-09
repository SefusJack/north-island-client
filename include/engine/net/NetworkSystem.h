#ifndef NORTH_ISLAND_NET_NETWORK_SYSTEM_H
#define NORTH_ISLAND_NET_NETWORK_SYSTEM_H

namespace net {

    /// @brief Starts the platform's socket library for as long as it lives (WSAStartup on Windows, nothing elsewhere); create one before any socket.
    class NetworkSystem {
        public:
            /// @brief Starts the socket library.
            NetworkSystem();

            /// @brief Shuts the socket library down.
            ~NetworkSystem();
            NetworkSystem(const NetworkSystem&) = delete;
            NetworkSystem& operator=(const NetworkSystem&) = delete;

            /// @brief Whether the socket library started.
            /// @return True when ready.
            bool isReady() const { return m_ready; }

        private:
            bool m_ready = false;
    };

}

#endif
