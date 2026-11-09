#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_WHEEL_PRIZES_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_WHEEL_PRIZES_H

namespace client {

    /// @brief The item, count and prize tier a wheel code stands for.
    /// @details Original: SetupWheelPrizes; codes 1..30 first prize, 31..60 second, 61..90 third, the rest ordinary.
    struct WheelPrize {
        int itemId = 0;
        int count = 0;
        int tier = 0;
    };

    /// @brief Looks up the prize of a code of the server's wheel list.
    /// @param code Wheel code from the server.
    /// @return The item, its count and tier (1 first, 2 second, 3 third, 0 ordinary).
    WheelPrize wheelPrizeOf(int code);

}

#endif
