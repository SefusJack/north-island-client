#include "protocol/Framing.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes frame(const core::Bytes& payload) {
        core::ByteWriter writer;
        writer.u8(kFrameMagic0).u8(kFrameMagic1).u16(static_cast<std::uint16_t>(payload.size())).bytes(payload);
        return writer.take();
    }

    void applyStreamKey(core::Bytes& bytes) {
        for (std::uint8_t& byte : bytes) {
            byte ^= kStreamKey;
        }
    }

}
