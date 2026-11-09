#ifndef NORTH_ISLAND_CLIENT_RULES_SOLDIER_TIME_H
#define NORTH_ISLAND_CLIENT_RULES_SOLDIER_TIME_H

#include <string>

namespace client {

    /// @brief The time a soldier is available, in the words of the game.
    /// @details Original: packet 15/24. The days, the hours (all of them, the days are not taken off), then "Again " and the minutes; nothing for no time.
    /// @param minutes The time in minutes.
    /// @return The text.
    std::string soldierTimeText(int minutes);

}

#endif
