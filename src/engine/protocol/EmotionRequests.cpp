#include "protocol/EmotionRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes emotionRequest(int emotionId) {
        core::ByteWriter writer;
        writer.u8(32).u8(1).u8(static_cast<std::uint8_t>(emotionId)); // 32/1: show an emotion (action 32)
        return writer.take();
    }

    core::Bytes actionRequest(int slot) {
        core::ByteWriter writer;
        writer.u8(32).u8(2).u8(static_cast<std::uint8_t>(slot)); // 32/2: play an action
        return writer.take();
    }

    core::Bytes stopActionRequest() {
        core::ByteWriter writer;
        writer.u8(32).u8(3); // 32/3: the action ended
        return writer.take();
    }

}
