#ifndef NORTH_ISLAND_PROTOCOL_PET_LISTS_H
#define NORTH_ISLAND_PROTOCOL_PET_LISTS_H

#include "core/Bytes.h"
#include "protocol/PetJoined.h"
#include "protocol/PetRecord.h"
#include "protocol/PetRide.h"
#include "protocol/PetsMounted.h"
#include "protocol/ScenePet.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes the own pets.
    /// @details Original: server 15/8 (RevInitFNpc, 2014 layout), records of 163 + name bytes.
    /// @param payload The packet payload.
    /// @return The pet records.
    std::vector<PetRecord> decodePetList(const core::Bytes& payload);

    /// @brief Decodes the pets stored at the inn, one record a place (slot is the place 1..10).
    /// @details Original: server 31/6 (RevInitINNFNpc, 2014 layout).
    /// @param payload The packet payload.
    /// @return The pet records.
    std::vector<PetRecord> decodeStoredPets(const core::Bytes& payload);

    /// @brief Decodes a pet that joined.
    /// @details Original: server 15/1 (RecvTeamPetAdded).
    /// @param payload The packet payload.
    /// @return The joined pet, or nothing when malformed.
    std::optional<PetJoined> decodePetJoined(const core::Bytes& payload);

    /// @brief Decodes other players' fighting pets.
    /// @details Original: server 15/4 (RecvScenePlayerPets).
    /// @param payload The packet payload.
    /// @return The scene pets.
    std::vector<ScenePet> decodeScenePets(const core::Bytes& payload);

    /// @brief Decodes the pets of a player of the scene.
    /// @details Original: server 15/7 (RecvPetMounted).
    /// @param payload The packet payload.
    /// @return The player's pets.
    PetsMounted decodePetsMounted(const core::Bytes& payload);

    /// @brief Decodes the players of the scene riding a pet; a place outside 1..4 ends the list.
    /// @details Original: server 23/116 (HandleRidePetListPacket).
    /// @param payload The packet payload.
    /// @return The rides.
    std::vector<PetRide> decodePetRides(const core::Bytes& payload);

}

#endif
