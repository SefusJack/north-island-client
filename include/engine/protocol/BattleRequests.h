#ifndef NORTH_ISLAND_PROTOCOL_BATTLE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_BATTLE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief Builds client 11/1: an event's battle result was reached, start the battle.
    /// @details Original: Inter_RunResults kind 3; the u8 is a debug form's value the server ignores.
    /// @return The encoded request.
    core::Bytes eventBattleRequest();

    /// @brief Builds client 11/1 3: the battle watched is left.
    /// @details Original: FFightResult 3 (QuitSpectating).
    /// @return The encoded request.
    core::Bytes quitViewingRequest();

    /// @brief Builds client 11/2: start or watch a battle (original SendServerMsg 11 2).
    /// @param kind 2 a PK on a player, 3 a map NPC, 4 watching the target's battle, 5 joining it.
    /// @param targetId Target id (i32).
    /// @param npcIndex Map NPC place (u16).
    /// @return The encoded request.
    core::Bytes battleRequest(int kind, std::int32_t targetId, int npcIndex);

    /// @brief Builds client 50/1: the fighter's command for the round (original SubmitActionOnTarget).
    /// @details Skill ids: attack 10001, defend 60021, flee 60041, capture 10008. Two noise bytes the server ignores follow the skill.
    /// @param actorRow Row of the actor.
    /// @param actorCol Column of the actor.
    /// @param targetRow Row of the target.
    /// @param targetCol Column of the target.
    /// @param skillId Skill id (u16).
    /// @param glazeSlot The glaze slot (u8).
    /// @return The encoded request.
    core::Bytes battleSkillRequest(
        int actorRow,
        int actorCol,
        int targetRow,
        int targetCol,
        int skillId,
        int glazeSlot
    );

    /// @brief Builds client 50/2: an item used in battle (like 50/1 with u16 item and u8 its bag place).
    /// @param actorRow Row of the actor.
    /// @param actorCol Column of the actor.
    /// @param targetRow Row of the target.
    /// @param targetCol Column of the target.
    /// @param itemId Item id (u16).
    /// @param bagSlot Bag place of the item (u8).
    /// @return The encoded request.
    core::Bytes battleItemRequest(
        int actorRow,
        int actorCol,
        int targetRow,
        int targetCol,
        int itemId,
        int bagSlot
    );

}

#endif
