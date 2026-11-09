#ifndef NORTH_ISLAND_FORMATS_ROLE_SPRITE_H
#define NORTH_ISLAND_FORMATS_ROLE_SPRITE_H

#include "core/Color.h"
#include "formats/RoleSpriteFrame.h"

#include <array>
#include <optional>
#include <vector>

namespace formats {

    /// @brief A character or equipment sprite: its palette and its frames, grouped into action slots.
    /// @details Walk directions are slots 0..7, standing 8..15, ... The frames of slot s follow those of slots 0..s-1.
    struct RoleSprite {
        std::array<core::Color, 256> palette{}; // 256 = entries of an 8-bit palette
        std::vector<RoleSpriteFrame> frames;
        std::vector<int> slotFrameCounts;

        /// @brief Number of frames in an action slot.
        /// @param slot Action slot.
        /// @return The frame count (0 for an unknown slot).
        int frameCount(int slot) const {
            return slot >= 0 && slot < static_cast<int>(slotFrameCounts.size()) ? slotFrameCounts[static_cast<std::size_t>(slot)] : 0;
        }

        /// @brief Index into frames of a slot's frame.
        /// @param slot Action slot.
        /// @param frame Frame number inside the slot.
        /// @return The index, or nothing when the sprite lacks that frame.
        std::optional<std::size_t> frameIndex(int slot, int frame) const {
            if (frame < 0 || frame >= frameCount(slot)) {
                return std::nullopt;
            }
            std::size_t index = 0;
            for (int s = 0; s < slot; ++s) {
                index += static_cast<std::size_t>(slotFrameCounts[static_cast<std::size_t>(s)]);
            }
            index += static_cast<std::size_t>(frame);
            if (index >= frames.size()) {
                return std::nullopt;
            }
            return index;
        }

        /// @brief Index of the first frame of the sprite's table, whichever action slot it belongs to.
        /// @details Original: Trole_show.LoadRoleFrame(id, 0, 0).
        /// @return 0, or nothing when the sprite has no frames.
        std::optional<std::size_t> firstFrameIndex() const { return frames.empty() ? std::nullopt : std::optional<std::size_t>(0); }
    };

}

#endif
