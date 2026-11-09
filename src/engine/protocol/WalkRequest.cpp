#include "protocol/WalkRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes WalkRequest::encode() const {
        core::ByteWriter writer;
        // 6 = walk action, sub id 2 = stopped, 1 = walking
        writer.u8(6).u8(stopped ? 2 : 1).u8(static_cast<std::uint8_t>(direction));
        writer.u16(static_cast<std::uint16_t>(destination.x)).u16(static_cast<std::uint16_t>(destination.y));
        writer.u8(check).u8(random);
        return writer.take();
    }

}
