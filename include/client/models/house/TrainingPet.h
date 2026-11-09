#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_TRAINING_PET_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_TRAINING_PET_H

#include "client/subsystems/minigames/HumanFrame.h"
#include "core/Point.h"
#include "formats/NpcInfo.h"

namespace client {

    /// @brief The pet that runs on the spot on a Treadmill piece.
    /// @details Original: DZ_Human.pas FUtensilPetHuman, set by UseUtensil from the owner's active pet.
    struct TrainingPet {
        formats::NpcInfo npc;
        int place = 0;
        int furnIndex = 0;
        core::Point world;
        int direction = 7;
        HumanFrame anim;
    };

}

#endif
