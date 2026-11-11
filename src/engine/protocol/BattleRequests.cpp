#include "protocol/BattleRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes eventBattleRequest() { return {11, 1, 0}; } // 11/1: event battle (action 11)

    core::Bytes quitViewingRequest() { return {11, 1, 3}; } // 11/1 with result 3: quit watching

    core::Bytes battleRequest(int kind, std::int32_t targetId, int npcIndex) {
        core::ByteWriter writer;
        // 11/2: start or watch a battle
        writer.u8(11).u8(2).u8(static_cast<std::uint8_t>(kind)).i32(targetId).u16(static_cast<std::uint16_t>(npcIndex));
        return writer.take();
    }

    core::Bytes battleSkillRequest(
        int actorRow,
        int actorCol,
        int targetRow,
        int targetCol,
        int skillId,
        int glazeSlot
    ) {
        core::ByteWriter writer;
        writer.u8(50).u8(1); // 50/1: skill command (action 50)
        writer.u8(static_cast<std::uint8_t>(actorRow)).u8(static_cast<std::uint8_t>(actorCol));
        writer.u8(static_cast<std::uint8_t>(targetRow)).u8(static_cast<std::uint8_t>(targetCol));
        writer.u16(static_cast<std::uint16_t>(skillId)).u8(0).u8(0).u8(static_cast<std::uint8_t>(glazeSlot));
        return writer.take();
    }

    core::Bytes battleItemRequest(
        int actorRow,
        int actorCol,
        int targetRow,
        int targetCol,
        int itemId,
        int bagSlot
    ) {
        core::ByteWriter writer;
        writer.u8(50).u8(2); // 50/2: item command (action 50)
        writer.u8(static_cast<std::uint8_t>(actorRow)).u8(static_cast<std::uint8_t>(actorCol));
        writer.u8(static_cast<std::uint8_t>(targetRow)).u8(static_cast<std::uint8_t>(targetCol));
        writer.u16(static_cast<std::uint16_t>(itemId)).u8(0).u8(0).u8(static_cast<std::uint8_t>(bagSlot));
        return writer.take();
    }

}
