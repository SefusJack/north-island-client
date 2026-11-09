#include "protocol/RebirthRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {
        constexpr std::uint8_t kRebirthAction = 44; // action 44: rebirth
    }

    core::Bytes petRebornRequest(int petPlace) {
        core::ByteWriter writer;
        writer.u8(kRebirthAction).u8(1).u8(static_cast<std::uint8_t>(petPlace)); // sub 1: reborn a pet
        return writer.take();
    }

}
