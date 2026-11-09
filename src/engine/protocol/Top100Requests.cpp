#include "protocol/Top100Requests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes top100Request(int kind, int page) {
        core::ByteWriter writer;
        writer.u8(84).u8(1).u8(static_cast<std::uint8_t>(kind)).u8(static_cast<std::uint8_t>(page)); // 84 = Top 100 action, 1 = list sub id
        return writer.take();
    }

}
