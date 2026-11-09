#ifndef NORTH_ISLAND_CLIENT_SERVICES_SERVERS_QOL_SERVERS_H
#define NORTH_ISLAND_CLIENT_SERVICES_SERVERS_QOL_SERVERS_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace client {

    /// @brief A server the player added on the server list by its address (an IP or a host name, and a port when not the game's 6414).
    struct DirectServer {
        std::string name;
        std::string host;
        std::uint16_t port = 6414; // the game's server port

        /// @brief Formats the address as host, or host:port when the port is not the game's.
        /// @return The address text.
        std::string address() const;

        /// @brief Compares name, host and port.
        /// @param other The server to compare with.
        /// @return True when all three match.
        bool operator==(const DirectServer& other) const { return name == other.name && host == other.host && port == other.port; }
    };

    /// @brief The saved-servers.json file of the user folder: the servers added by address (SERVER.INI is left as it is) and the server list's collapsed sections ("direct", "list", "group:<number>").
    struct SavedServers {
        std::vector<DirectServer> direct;
        std::set<std::string> collapsed;

        /// @brief Gives the path of the servers file.
        /// @param userRoot The user folder.
        /// @return The path of saved-servers.json in it.
        static std::filesystem::path fileIn(const std::filesystem::path& userRoot);

        /// @brief Reads the servers file, or the old qol-servers.json when there is none yet.
        /// @param userRoot The user folder.
        /// @return The file's servers; none (and nothing collapsed) without one or when it cannot be read.
        static SavedServers loadIn(const std::filesystem::path& userRoot);

        /// @brief Parses the servers file's JSON text.
        /// @param json The text.
        /// @return The servers, or nothing when the text is not a JSON object.
        static std::optional<SavedServers> parse(const std::string& json);

        /// @brief Writes the servers as JSON text.
        /// @return The text.
        std::string toJson() const;

        /// @brief Saves the servers to a file, creating its folder.
        /// @param file The file to write.
        /// @return True when the file was written.
        bool save(const std::filesystem::path& file) const;

        /// @brief Parses "host" or "host:port": a host name or IPv4 address (letters, digits, '.', '-') and a port 1..65535.
        /// @param text The address text.
        /// @return The server (named after its address), or nothing when the text is not valid.
        static std::optional<DirectServer> parseAddress(const std::string& text);

        /// @brief Compares the direct servers and the collapsed sections.
        /// @param other The servers to compare with.
        /// @return True when both match.
        bool operator==(const SavedServers& other) const { return direct == other.direct && collapsed == other.collapsed; }
    };

}

#endif
