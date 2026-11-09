#ifndef NORTH_ISLAND_CLIENT_DATA_STAR_INTRODUCTIONS_H
#define NORTH_ISLAND_CLIENT_DATA_STAR_INTRODUCTIONS_H

#include <string>

namespace client {

    /// @brief The JRole picture the introduction of a star shows (the own star shows the player's character).
    /// @details Original: OpenStarRoleInfo.
    /// @param star Star number.
    /// @return Picture name, "" for none.
    std::string starIntroduction(int star);

}

#endif
