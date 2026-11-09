#ifndef NORTH_ISLAND_CLIENT_RULES_DISCONNECT_NOTICE_H
#define NORTH_ISLAND_CLIENT_RULES_DISCONNECT_NOTICE_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief What the form of a disconnection says and offers.
    /// @details Original: DZ_Connect.pas action 0 (the server sends its reason and ends the session).
    struct DisconnectNotice {
        /// @brief Buttons of the form: Prev. and Quit (back to the server list, or leave), Update and Quit (the edition is old), or only End.
        enum class Buttons { PreviousAndQuit, UpdateAndQuit, End };

        std::string text;
        Buttons buttons = Buttons::End;
    };

    /// @brief Builds the notice of a disconnect reason: the reason's own words behind its number for the game's own ids and for the reasons that name themselves, else only "Disconnection:" and the number.
    /// @details Original: 0/reason, DZ_Connect.pas action 0.
    /// @param reason The reason sent by the server.
    /// @param playerId Player.FHumId.
    /// @return The text and buttons of the form.
    DisconnectNotice disconnectNotice(int reason, std::int32_t playerId);

}

#endif
