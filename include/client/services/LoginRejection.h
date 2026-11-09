#ifndef NORTH_ISLAND_CLIENT_SERVICES_LOGIN_REJECTION_H
#define NORTH_ISLAND_CLIENT_SERVICES_LOGIN_REJECTION_H

namespace client {

    /// @brief Why the server refused a login (server 1/5, 1/6, 1/7).
    enum class LoginRejection { Timeout, WrongPassword, ConnectionFailed };

}

#endif
