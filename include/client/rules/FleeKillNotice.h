#ifndef NORTH_ISLAND_CLIENT_RULES_FLEE_KILL_NOTICE_H
#define NORTH_ISLAND_CLIENT_RULES_FLEE_KILL_NOTICE_H

#include <optional>
#include <string>

namespace client {

    /// @brief A notice of the flee-killing event: a chat line said by the player himself or a short message for durationMs.
    /// @details Original: MZ_Menu.pas RecvFleeKillEvent (77/1). The chat line is AddTalkMessage with Player.FHumId; some of the refusals also put the mouse cursor back to the plain arrow (SetMouseCursorType 0).
    struct FleeKillNotice {
        bool chat = false;
        int durationMs = 0;
        bool resetsCursor = false;
        std::string text;
    };

    /// @brief Gives the notice of a flee-killing event code 1..15. Code 1 begins the event (FFleeKillActive 1) and code 3 ends it (0).
    /// @details Original: MZ_Menu.pas RecvFleeKillEvent, 77/1.
    /// @param code The event code.
    /// @return The notice, or nothing for any other code.
    std::optional<FleeKillNotice> fleeKillNotice(int code);

}

#endif