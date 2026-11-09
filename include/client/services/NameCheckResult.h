#ifndef NORTH_ISLAND_CLIENT_SERVICES_NAME_CHECK_RESULT_H
#define NORTH_ISLAND_CLIENT_SERVICES_NAME_CHECK_RESULT_H

namespace client {

    /// @brief The server's answer to a new character's name (server 9/3 code 0, 1, 2).
    enum class NameCheckResult { Accepted, Unavailable, Invalid };

}

#endif
