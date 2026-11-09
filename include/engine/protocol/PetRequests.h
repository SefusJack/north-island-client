#ifndef NORTH_ISLAND_PROTOCOL_PET_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_PET_REQUESTS_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Builds 19/1: the pet joins battles (and follows on the map).
    /// @param npcId Pet npc id (i32).
    /// @return The encoded payload.
    core::Bytes petFightRequest(int npcId);

    /// @brief Builds 19/2: the fighting pet rests.
    /// @return The encoded payload.
    core::Bytes petRestRequest();

    /// @brief Builds 15/6: rename a pet.
    /// @param place Pet place (u8).
    /// @param name New name (the rest of the message).
    /// @return The encoded payload.
    core::Bytes renamePetRequest(int place, const std::string& name);

    /// @brief Builds 15/2: send a pet away.
    /// @param place Pet place (u8).
    /// @return The encoded payload.
    core::Bytes exilePetRequest(int place);

    /// @brief Builds 28/1: the skill window's arrow learns a skill of the pet shown (the server takes level 1 for the pet's skills).
    /// @param skillId Skill id (u16).
    /// @param level Skill level (sent as u8 level + 1).
    /// @return The encoded payload.
    core::Bytes learnSkillRequest(int skillId, int level);

    /// @brief Builds 15/11: ride a pet.
    /// @param place Pet place (u8).
    /// @param npcId Pet npc id (i32).
    /// @return The encoded payload.
    core::Bytes ridePetRequest(int place, int npcId);

    /// @brief Builds 15/12: get off a ridden pet.
    /// @param place Pet place (u8).
    /// @param npcId Pet npc id (i32).
    /// @return The encoded payload.
    core::Bytes dismountPetRequest(int place, int npcId);

}

#endif
