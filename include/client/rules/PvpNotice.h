#ifndef NORTH_ISLAND_CLIENT_RULES_PVP_NOTICE_H
#define NORTH_ISLAND_CLIENT_RULES_PVP_NOTICE_H

#include <optional>
#include <string>

namespace client {

    /// @brief A notice of a PvP event: a chat line (AddTalkMessage, channel 0) or a short message in the middle of the screen (ShowShortMsg, for durationMs).
    struct PvpNotice {
        bool chat = false;
        int durationMs = 0;
        std::string text;
    };

    /// @brief The notice for a PvP event code (Team PK, the PvP Server, Group PvP).
    /// @details Original: ShowPvpEventNotice (packet 69/1).
    /// @param code The event code.
    /// @return The notice, none for a code the game does not word.
    std::optional<PvpNotice> pvpEventNotice(int code);

    /// @brief The notice for a 4 player PK code.
    /// @details Original: Show4PlayerPkNotice (packet 69/3).
    /// @param code The event code.
    /// @return The notice, none for a code the game does not word.
    std::optional<PvpNotice> fourPlayerPkNotice(int code);

    /// @brief The notice for an arena code.
    /// @details Original: RecvArenaNotice (packet 37/3).
    /// @param code The event code.
    /// @return The notice, none for a code the game does not word.
    std::optional<PvpNotice> arenaNotice(int code);

}

#endif
