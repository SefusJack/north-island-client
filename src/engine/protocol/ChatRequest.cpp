#include "protocol/ChatRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes chatRequest(int sub, const std::string& text) {
        core::ByteWriter writer;
        writer.u8(2).u8(static_cast<std::uint8_t>(sub)).text(text); // action 2: chat
        return writer.take();
    }

    core::Bytes whisperRequest(std::int32_t targetId, const std::string& text) {
        core::ByteWriter writer;
        writer.u8(2).u8(3).i32(targetId).text(text); // 2/3: whisper
        return writer.take();
    }

}
