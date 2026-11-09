#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_ROLE_LOOK_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_ROLE_LOOK_H

#include "client/models/character/CharacterLook.h"
#include "formats/NpcInfo.h"

#include <variant>

namespace client {

    using MovieRoleLook = std::variant<CharacterLook, formats::NpcInfo>;

}

#endif
