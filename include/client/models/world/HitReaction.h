#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_HIT_REACTION_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_HIT_REACTION_H

#include "core/Point.h"
#include "formats/ColorPattern.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief What shows on a player the impact of a thrown item hit, until he leaves the cell he stood in.
    /// @details Original: FEmoteFx, FEmoteFxFrame, FEmoteFxPosX / Y (HandlePlayerEffectArea). Kind 1: the 9081 sprite over his head
    /// in its frame; kind 2: FEmoteFx 2, the figure drawn with its colour patterns cleared.
    struct HitReaction {
        int kind = 0;
        int frame = 0;
        core::Point at;

        /// @brief Builds the colour pattern a kind 2 hit draws the figure with.
        /// @details Original: DrawHumanLayers, FEmoteFx 2: the 45 colour pattern digits cleared to 0 but the first, 1.
        /// @return The cleared pattern.
        static formats::ColorPattern clearedPattern() {
            formats::ColorPattern cleared(std::array<std::uint8_t, formats::ColorPattern::kSize>{});
            cleared.setDigit(0, 1);
            return cleared;
        }
    };

}

#endif
