#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_GROUND_OBJECT_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_GROUND_OBJECT_H

#include "core/Point.h"
#include "formats/GroundObjectInfo.h"
#include "gfx/PictureId.h"

#include <cstdint>

namespace client {

    /// @brief A ground object on the loaded map: what it is, its picture, where its top-left corner lies (map pixels) and its animation.
    /// @details Original: TGroundObj.
    struct GroundObject {
        formats::GroundObjectInfo info;
        gfx::PictureId picture;
        core::Point position;
        bool light = false;
        int frame = 0;
        std::uint64_t frameMs = 0;

        /// @brief The map row it stands on, for sorting with the characters.
        /// @return Position y plus the object's depth.
        int footY() const { return position.y + info.depth; }
    };

}

#endif
