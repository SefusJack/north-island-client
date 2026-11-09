#ifndef NORTH_ISLAND_PROTOCOL_BATTLE_LISTS_H
#define NORTH_ISLAND_PROTOCOL_BATTLE_LISTS_H

#include "core/Bytes.h"
#include "protocol/BattleAction.h"
#include "protocol/BattleStart.h"
#include "protocol/FighterWeaponLook.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes server 11/250: the battle ground, the event's battle and the fighters.
    /// @details Original: RevFightStart.
    /// @param payload Packet payload.
    /// @return The battle start, or nothing when malformed.
    std::optional<BattleStart> decodeBattleStart(const core::Bytes& payload);

    /// @brief Decodes server 11/5: u8 state for all, then 29-byte fighter records of fighters entering a running battle.
    /// @details Original: SupportRoleInfo. The records are the 11/250 record without its state.
    /// @param payload Packet payload.
    /// @return The joining fighters.
    std::vector<FighterRecord> decodeBattleJoin(const core::Bytes& payload);

    /// @brief Decodes server 11/251: u8 0, then 12-byte records (i32 id, i32 owner, u8 row, u8 col, u16 item).
    /// @details Original: RecvFighterEquipLook.
    /// @param payload Packet payload.
    /// @return The fighters' weapon looks.
    std::vector<FighterWeaponLook> decodeFighterWeaponLooks(const core::Bytes& payload);

    /// @brief Decodes server 50/1: the actions played together.
    /// @details Original: RevAttackSkill.
    /// @param payload Packet payload.
    /// @return The actions; none when the packet is cut short.
    std::vector<BattleAction> decodeBattleActions(const core::Bytes& payload);

}

#endif
