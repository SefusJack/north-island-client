#include "protocol/LeaveWordRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {
        constexpr std::uint8_t kLeaveWordAction = 36; // action 36: announcement / leave-word forms
    }

    core::Bytes leaveWordSendRequest(const std::string& text) {
        core::ByteWriter writer;
        writer.u8(kLeaveWordAction).u8(1).text(text); // sub 1: leave a message
        return writer.take();
    }

    core::Bytes leaveWordClearRequest(std::uint16_t number) {
        core::ByteWriter writer;
        writer.u8(kLeaveWordAction).u8(2).u16(number); // sub 2: take a message away
        return writer.take();
    }

}
