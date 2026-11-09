#ifndef NORTH_ISLAND_CLIENT_ACTIONS_EVENT_NOTICE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_EVENT_NOTICE_H

#include <functional>
#include <optional>
#include <string_view>

namespace client {

    /// @brief One event notice text of server action 20 (subs 24 to 61, DZ_Connect.pas) and how it is shown.
    /// @details Shown either as a short message at the top for durationMs or as a chat line of the own player (AddTalkMessage on the system channel).
    struct EventNotice {
        /// @brief How a notice is shown: a short top message or a chat line.
        enum class Kind { Short, Chat };

        int sub = 0;
        int choice = -1;
        Kind kind = Kind::Short;
        int durationMs = 0;
        std::string_view text;

        /// @brief Finds the notice of a sub with the choice the message carries.
        /// @param sub Sub id of action 20 (24..61).
        /// @param choice The message's second byte.
        /// @return The notice, or nullopt when the sub is not a plain notice.
        static std::optional<std::reference_wrapper<const EventNotice>> find(int sub, int choice);
    };

}

#endif
