#include "formats/GroundObjectInfo.h"

#include "core/ByteReader.h"

#include <algorithm>

namespace formats {

    namespace {

        constexpr std::size_t kSizeWithoutFirstFrame = 21; // record bytes without the optional first-frame byte: i32 + i32 + i32 + u8 + 3 * u16 + 2 * u8
        constexpr int kDepthCell = 20; // map cell size, px: the stored depth is in cells

    }

    std::optional<GroundObjectInfo> GroundObjectInfo::parse(const core::Bytes& data) {
        if (data.size() < kSizeWithoutFirstFrame) {
            return std::nullopt;
        }
        core::ByteReader reader(data);
        GroundObjectInfo info;
        info.pictureId = reader.i32();
        reader.skip(4); // skips the i32 value field
        info.depth = reader.i32() * kDepthCell;
        info.frames = std::max(1, static_cast<int>(reader.u8()));
        info.frameMs = reader.u16();
        info.width = reader.u16();
        info.height = reader.u16();
        info.alpha = reader.u8();
        info.kind = reader.u8();
        if (!reader.atEnd()) {
            info.firstFrame = reader.u8();
        }
        const std::int32_t id = info.pictureId;
        // 11011..11015 and 11031: picture ids forced opaque (255) and sorted with the characters (kind 3)
        if ((id >= 11011 && id <= 11015) || id == 11031) {
            info.alpha = 255; // fully opaque
            info.kind = 3; // object kind 3: sorted with the characters
        }
        return info;
    }

}
