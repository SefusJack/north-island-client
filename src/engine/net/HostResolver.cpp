#include "net/HostResolver.h"

#include "Platform.h"

#include <algorithm>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace net {

    namespace {

        /// @brief Writes an IPv4 socket address as dotted text.
        /// @param address The socket address (an IPv4 one).
        /// @return The text a.b.c.d.
        std::string ipv4Text(const sockaddr* address) {
            const auto* bytes = reinterpret_cast<const unsigned char*>(&reinterpret_cast<const sockaddr_in*>(address)->sin_addr);
            return std::to_string(bytes[0]) + "." + std::to_string(bytes[1]) + "." + std::to_string(bytes[2]) + "." + std::to_string(bytes[3]);
        }

        /// @brief Blocks while resolving a host name to its first IPv4 address.
        /// @param host The host name.
        /// @return The dotted address, or nothing when the lookup failed.
        std::optional<std::string> lookUp(const std::string& host) {
            addrinfo hints{};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            addrinfo* found = nullptr;
            if (getaddrinfo(host.c_str(), nullptr, &hints, &found) != 0 || found == nullptr) {
                return std::nullopt;
            }
            std::string text = ipv4Text(found->ai_addr);
            freeaddrinfo(found);
            return text;
        }

    }

    /// @brief One lookup: its result guarded by a mutex because a thread writes it.
    struct HostResolver::Lookup {
        mutable std::mutex mutex;
        Result result{State::Pending, {}};
    };

    void HostResolver::request(const std::string& host) {
        if (const auto known = m_lookups.find(host); known != m_lookups.end() && result(host).state != State::Failed) {
            return;
        }
        auto lookup = std::make_shared<Lookup>();
        m_lookups[host] = lookup;
        if (isIPv4Address(host)) {
            lookup->result = {State::Found, host};
            return;
        }
        std::thread([lookup, host] {
            const std::optional<std::string> address = lookUp(host);
            const std::lock_guard<std::mutex> lock(lookup->mutex);
            lookup->result = address ? Result{State::Found, *address} : Result{State::Failed, {}};
        }).detach();
    }

    HostResolver::Result HostResolver::result(const std::string& host) const {
        const auto found = m_lookups.find(host);
        if (found == m_lookups.end()) {
            return {};
        }
        const std::lock_guard<std::mutex> lock(found->second->mutex);
        return found->second->result;
    }

    bool isIPv4Address(const std::string& text) {
        int parts = 0;
        std::size_t start = 0;
        while (start <= text.size()) {
            const std::size_t dot = (std::min)(text.find('.', start), text.size());
            const std::string part = text.substr(start, dot - start);
            // a part has at most 3 digits and is at most 255
            if (part.empty() || part.size() > 3 || part.find_first_not_of("0123456789") != std::string::npos || std::stoi(part) > 255) {
                return false;
            }
            ++parts;
            start = dot + 1;
        }
        return parts == 4; // an IPv4 address has four parts
    }

}
