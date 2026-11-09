#include "protocol/SandglassMessages.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture
        constexpr std::uint8_t kSandglass = 53; // 62/53: sandglass
        constexpr int kUsedKind = 1;
        constexpr int kFlagsKind = 2;

    }

    core::Bytes sandglassRequest(int piece) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(kSandglass).u8(kUsedKind).u8(static_cast<std::uint8_t>(piece));
        return writer.take();
    }

    std::optional<SandglassUsed> decodeSandglassUsed(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        if (reader.u8() != kUsedKind) {
            return std::nullopt;
        }
        SandglassUsed used;
        used.piece = reader.u8();
        used.value = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return used;
    }

    std::optional<int> decodeSandglassFlags(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        if (reader.u8() != kFlagsKind) {
            return std::nullopt;
        }
        const int flags = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return flags;
    }

}
