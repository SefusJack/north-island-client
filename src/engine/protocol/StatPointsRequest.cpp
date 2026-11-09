#include "protocol/StatPointsRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes statPointsRequest(const std::vector<std::pair<int, std::int32_t>>& points) {
        core::ByteWriter writer;
        writer.u8(8).u8(1).u8(0).u8(static_cast<std::uint8_t>(points.size())); // 8 = stats action, 1 = spend points sub id
        for (const auto& [attribute, amount] : points) {
            writer.u8(static_cast<std::uint8_t>(attribute)).i32(amount);
        }
        return writer.take();
    }

}
