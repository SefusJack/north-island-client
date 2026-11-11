#ifndef NORTH_ISLAND_PROTOCOL_HOT_KEY_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_HOT_KEY_REQUESTS_H

#include "core/Bytes.h"
#include "protocol/HotKeyRecord.h"

#include <cstdint>
#include <vector>

namespace protocol {

    /// @brief Decodes server 40/1: the bar's places, 5-byte records.
    /// @param payload Packet payload.
    /// @return The bar's records.
    std::vector<HotKeyRecord> decodeHotKeys(const core::Bytes& payload);

    /// @brief Builds client 40/1: what a place of the bar holds now (original SendHotKeySlot).
    /// @param record The place and its content (u8 kind, u16 id, u8 page, u8 place).
    /// @return The encoded request.
    core::Bytes hotKeyRequest(const HotKeyRecord& record);

    /// @brief Builds client 23/20: a bar item or healing skill used in the world (original UseTargetOnSelf / UseTargetOnPet).
    /// @param kind 1 item, 2 skill.
    /// @param id Item or skill id (u16).
    /// @param casterPetSlot Pet place the skill comes from (u8); 0 is the player's.
    /// @param targetKind 2 the player, 4 a pet.
    /// @param targetSlot The target pet's place (u8).
    /// @param targetId The target pet's id (i32).
    /// @return The encoded request.
    core::Bytes hotKeyUseRequest(
        int kind,
        int id,
        int casterPetSlot,
        int targetKind,
        int targetSlot,
        std::int32_t targetId
    );

}

#endif
