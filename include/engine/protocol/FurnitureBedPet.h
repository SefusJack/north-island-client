#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_BED_PET_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_BED_PET_H

#include "protocol/PetRecord.h"

namespace protocol {

    /// @brief A pet put to sleep in a bed of a house (server 62/43): the piece, the bed in it (1..4), the number it is kept under (1..6) and the pet.
    /// @details Original: HandleUtensilPacket; the pet is the 339-byte TStoredPetRec.
    struct FurnitureBedPet {
        int furnIndex = 0;
        int bed = 0;
        int id = 0;
        PetRecord pet;
    };

}

#endif
