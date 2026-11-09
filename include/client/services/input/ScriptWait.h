#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_WAIT_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_SCRIPT_WAIT_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief A script command that holds the script until something is so (walkto, waitwin, waitmsg, waitmap) or its time is up.
    struct ScriptWait {
        /// @brief What is waited for: the end of a walk, a window, a message line or a map.
        enum class Kind { Walk, Window, Message, Map };

        Kind kind = Kind::Walk;
        std::string what;
        std::string text;
        int number = 0;
        std::uint64_t mark = 0;
        std::uint64_t startMs = 0;
        std::uint64_t deadlineMs = 0;
    };

}

#endif
