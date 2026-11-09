#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_CAREER_MODEL_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_CAREER_MODEL_H

#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"

#include <string>

namespace client {

    /// @brief A figure the career forms show: the player in the clothes of a class, its pose and animation.
    /// @details Original: TShowModel.
    struct CareerModel {
        std::string name;
        int career = 0;
        CharacterLook look;
        int pose = 16; // default pose slot
        FigureAnimation animation;
    };

}

#endif
