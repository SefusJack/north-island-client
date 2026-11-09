#ifndef NORTH_ISLAND_PROTOCOL_FURNITURE_BED_MESSAGES_H
#define NORTH_ISLAND_PROTOCOL_FURNITURE_BED_MESSAGES_H

#include "core/Bytes.h"
#include "protocol/FurnitureBedPet.h"

#include <optional>

namespace protocol {

    /// @brief Decodes server 62/43: a pet put to bed (u16 piece, u8 bed, u8 number, the 339-byte stored pet).
    /// @details Original: TStoredPetRec.
    /// @param payload Packet payload.
    /// @return The bed pet, or nothing when malformed.
    std::optional<FurnitureBedPet> decodeFurnitureBedPet(const core::Bytes& payload);

    /// @brief Builds client 62/43: the pet goes to bed.
    /// @param piece Furniture piece (u16).
    /// @param petPlace The pet's place in the team (u8).
    /// @return The encoded request.
    core::Bytes putPetInBedRequest(int piece, int petPlace);

    /// @brief Builds client 62/44: the pet in the bed comes back.
    /// @param piece Furniture piece (u16).
    /// @param bed Bed (u8).
    /// @return The encoded request.
    core::Bytes takePetFromBedRequest(int piece, int bed);

}

#endif
