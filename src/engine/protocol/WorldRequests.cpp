#include "protocol/WorldRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes doorEnteredRequest(int doorIndex) {
        core::ByteWriter writer;
        // 20 = event action, 8 = door entered sub id
        writer.u8(20).u8(8).u16(static_cast<std::uint16_t>(doorIndex));
        return writer.take();
    }

    core::Bytes npcClickedRequest(int npcIndex) {
        core::ByteWriter writer;
        // 20 = event action, 1 = NPC clicked sub id
        writer.u8(20).u8(1).u16(static_cast<std::uint16_t>(npcIndex));
        return writer.take();
    }

    core::Bytes npcNearRequest(int npcIndex) {
        core::ByteWriter writer;
        // 20 = event action, 2 = NPC near sub id
        writer.u8(20).u8(2).u16(static_cast<std::uint16_t>(npcIndex));
        return writer.take();
    }

    core::Bytes eventAnswerRequest(int answer) { return {20, 9, static_cast<std::uint8_t>(answer)}; } // 20 = event action, 9 = event answer sub id

    core::Bytes nextEventResultRequest() { return {20, 6}; } // 20 = event action, 6 = next event result sub id

    core::Bytes mapLoadedRequest() { return {12, 1}; } // 12 = map action, 1 = map loaded sub id

}
