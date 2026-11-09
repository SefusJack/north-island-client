#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GOBANG_SEAT_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GOBANG_SEAT_H

#include "client/models/character/CharacterLook.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief A player at a seat of the Five in a Row hall as the hall draws him (original THuman made from the scene's players, no weapon); id 0 is nobody.
    struct GobangSeat {
        std::int32_t id = 0;
        std::string name;
        CharacterLook look;
    };

}

#endif
