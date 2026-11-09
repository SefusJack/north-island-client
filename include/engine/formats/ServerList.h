#ifndef NORTH_ISLAND_FORMATS_SERVER_LIST_H
#define NORTH_ISLAND_FORMATS_SERVER_LIST_H

#include "formats/ServerEntry.h"
#include "formats/ServerGroup.h"

#include <optional>
#include <string>
#include <vector>

namespace formats {

    /// @brief The server groups parsed from SERVER.INI, in group-number order, plus the separate PvP server.
    class ServerList {
        public:
            static constexpr int kPvpGroup = 95; // SERVER.INI group number of the separate PvP server

            /// @brief Parses the lines of SERVER.INI into groups and servers.
            /// @param lines The text lines of the file.
            /// @return The parsed list (group 95 is kept apart as the PvP server).
            static ServerList parse(const std::vector<std::string>& lines);

            /// @brief The listed server groups in group-number order.
            /// @return The groups, without the PvP group.
            const std::vector<ServerGroup>& groups() const { return m_groups; }

            /// @brief The PvP server (group 95), which is not listed with the other groups.
            /// @return The PvP server, or nothing when the file has none.
            const std::optional<ServerEntry>& pvpServer() const { return m_pvpServer; }

        private:
            std::vector<ServerGroup> m_groups;
            std::optional<ServerEntry> m_pvpServer;
    };

}

#endif
