#ifndef NORTH_ISLAND_CLIENT_SERVICES_SERVER_RELOGIN_H
#define NORTH_ISLAND_CLIENT_SERVICES_SERVER_RELOGIN_H

#include <cstdint>
#include <string>

namespace client {

    /// @brief State of the automatic relogin (account, character slot, delay, PvP flag) that the world hands to the login scene.
    /// @details Original: ScheduleServerRelogin and the Se_SystemForm.FRelogin* fields.
    struct ServerRelogin {
        std::string account;
        std::string password;
        int slot = 1;
        std::string address;
        std::string lineName;
        std::uint64_t delayMs = 3000; // ms to wait before logging in again
        bool toPvp = false;
    };

}

#endif
