#ifndef NORTH_ISLAND_FORMATS_GROUND_OBJECT_INFO_H
#define NORTH_ISLAND_FORMATS_GROUND_OBJECT_INFO_H

#include "core/Bytes.h"

#include <cstdint>
#include <optional>

namespace formats {

    /// @brief A ground object's picture, animation and drawing data.
    /// @details Original: Wem.MMG "<objectId>.wem", 22 bytes: i32 picture, i32 value, i32 depth (cells), u8 frames, u16 frame time (ms),
    /// u16 width, u16 height, u8 alpha (255 = opaque), u8 kind, [u8 first frame]. Pictures are "<pictureId>" in the bitmap archives, frames stacked vertically.
    /// Kinds: below 3 lie flat under everything, 3 are sorted with the characters by their foot (top + depth), above 3 lie over everything.
    struct GroundObjectInfo {
        std::int32_t pictureId = 0;
        int depth = 0;
        int frames = 1;
        int frameMs = 0;
        int width = 0;
        int height = 0;
        int alpha = 255; // 255 = fully opaque
        int kind = 0;
        int firstFrame = 0;

        /// @brief Parses one .wem record.
        /// @param data Raw record bytes.
        /// @return The info, or nothing when the record is too short.
        static std::optional<GroundObjectInfo> parse(const core::Bytes& data);
    };

}

#endif
