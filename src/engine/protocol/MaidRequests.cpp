#include "protocol/MaidRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes maidRequest(int sub, int furnIndex, const core::Bytes& costume) {
        constexpr std::uint8_t kMaid = 76; // action 76: maid
        core::ByteWriter writer;
        writer.u8(kMaid).u8(static_cast<std::uint8_t>(sub)).u8(static_cast<std::uint8_t>(furnIndex));
        for (const std::uint8_t byte : costume) {
            writer.u8(byte);
        }
        return writer.take();
    }

}
