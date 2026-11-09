#include "protocol/ExchangeRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes pointExchangeRequest(std::int32_t points) {
        core::ByteWriter writer;
        writer.u8(23).u8(109).i32(points); // 23/109: exchange IM points for gold (action 23)
        return writer.take();
    }

}
