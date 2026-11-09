#include "protocol/ActivityRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kActivities = 89; // client action 89: event schedule requests

    }

    core::Bytes activityListRequest(std::int32_t version) {
        core::ByteWriter writer;
        writer.u8(kActivities).u8(0).i32(version);
        return writer.take();
    }

    core::Bytes activityTeleportRequest(int number) {
        core::ByteWriter writer;
        writer.u8(kActivities).u8(1).u8(static_cast<std::uint8_t>(number));
        return writer.take();
    }

}
