#ifndef NORTH_ISLAND_CLIENT_SERVICES_ACCOUNT_SESSION_H
#define NORTH_ISLAND_CLIENT_SERVICES_ACCOUNT_SESSION_H

#include "client/services/ServerRelogin.h"
#include "protocol/CharacterSummary.h"
#include "protocol/OwnPlayerRecord.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief What the client knows about the logged-in account between screens.
    struct AccountSession {
        std::uint16_t serverId = 0;
        std::string serverName;
        bool serverGrouped = false;
        std::string serverLineName;
        std::string serverAddress;
        int serverPort = 0;
        std::string loginName;
        std::string password;
        std::optional<ServerRelogin> relogin;
        bool onPvpServer = false;
        std::uint32_t loginId = 0;
        std::vector<protocol::CharacterSummary> characters;
        int playingSlot = 0;
        std::optional<protocol::OwnPlayerRecord> ownPlayer;

        /// @brief Finds the created character in a slot.
        /// @param slot Character slot number.
        /// @return The character, or nothing when the slot is empty (level 0) or unknown.
        std::optional<protocol::CharacterSummary> character(int slot) const {
            for (const auto& c : characters) {
                if (c.slot == slot && c.level != 0) {
                    return c;
                }
            }
            return std::nullopt;
        }
    };

}

#endif
