#ifndef NORTH_ISLAND_CLIENT_RULES_PVP_SERVER_NAMES_H
#define NORTH_ISLAND_CLIENT_RULES_PVP_SERVER_NAMES_H

#include <string>

namespace client {

    /// @brief A player's name as shown on the PvP server ("N-Name", N the player's server number).
    /// @details Original: Se_SystemForm.FOnPvpServer; FServerCode is 0 until packet 87/3 says otherwise; used by DrawHumInfo, GetPlayerNameForMessage and the Abstain notice of RevFightAbstain.
    /// @param name The player's name.
    /// @param serverCode The player's server number.
    /// @param onPvpServer Whether the client is on the PvP server.
    /// @return The shown name; as it is when not on the PvP server.
    std::string pvpServerPlayerName(const std::string& name, int serverCode, bool onPvpServer);

}

#endif
