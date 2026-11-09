#include "protocol/ExpressRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kTrade = 25; // action 25: trade (also carries the pet express)

    }

    core::Bytes expressStartRequest(std::int32_t petId, std::int32_t targetId, const std::vector<std::pair<int, int>>& items) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(20).i32(petId).i32(targetId); // 25/20: send the pet with items
        for (const auto& [slot, count] : items) {
            writer.u8(static_cast<std::uint8_t>(slot)).u8(static_cast<std::uint8_t>(count));
        }
        return writer.take();
    }

    core::Bytes expressStopRequest() { return {kTrade, 21}; } // 25/21: call the pet back

}
