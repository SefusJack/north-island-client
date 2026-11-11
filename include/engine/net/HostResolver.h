#ifndef NORTH_ISLAND_NET_HOST_RESOLVER_H
#define NORTH_ISLAND_NET_HOST_RESOLVER_H

#include <map>
#include <memory>
#include <string>

namespace net {

    /// @brief Looks host names up away from the game's thread (getaddrinfo can take seconds): ask for a name, then poll for its IPv4 address.
    /// @details A dotted address is found at once. A lookup still running when the resolver goes is left to finish on its own.
    class HostResolver {
        public:
            /// @brief How far a lookup is.
            enum class State { Unknown, Pending, Found, Failed };

            /// @brief The state of a lookup and, once found, its address.
            struct Result {
                State state = State::Unknown;
                std::string address;
            };

            /// @brief Starts a lookup of the name unless one was started (a failed one is tried again).
            /// @param host Host name or dotted address.
            void request(const std::string& host);

            /// @brief The state of the lookup of a name.
            /// @param host Host name or dotted address.
            /// @return The state and address; Unknown when never requested.
            Result result(const std::string& host) const;

        private:
            /// @brief One running or finished lookup, shared with its thread.
            struct Lookup;
            std::map<std::string, std::shared_ptr<Lookup>> m_lookups;
    };

    /// @brief Tells whether text is a dotted IPv4 address (a.b.c.d with each part 0..255).
    /// @param text The text to check.
    /// @return True for a dotted IPv4 address.
    bool isIPv4Address(const std::string& text);

}

#endif
