#ifndef NORTH_ISLAND_CLIENT_DATA_STARTER_ROLE_H
#define NORTH_ISLAND_CLIENT_DATA_STARTER_ROLE_H

#include "core/Point.h"

#include <string>

namespace client {

    /// @brief One of the fourteen characters a new player picks from (picture Btn_RolePic_N in the creation wizard).
    /// @details Holds the body type and head it gives, and how the points page shows it: the description picture and
    /// its position, and the role picture's bottom-centre position.
    struct StarterRole {
        int bodyType = 1;
        int head = 0;
        std::string infoPicture;
        core::Point infoPosition;
        core::Point picturePosition;
    };

}

#endif
