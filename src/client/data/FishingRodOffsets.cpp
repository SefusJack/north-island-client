#include "client/data/FishingRodOffsets.h"

namespace client {

    namespace {

        constexpr int kBodyTypes = 5; // body types 0..4 (one table row each)
        constexpr int kFirstSlot = 45; // animation slot just before the first fishing sit slot (46); table index = slot - 45

        // x offsets in pixels: 5 body types, 9 entries (index 0 unused, 1..8 are the slots 46..53)
        constexpr int kOffsetX[kBodyTypes][9] = {
            {0, -14, 12, 12, 16, 40, -22, -15, -14},
            {0, -14, 12, 12, 24, 28, -22, -15, -14},
            {0, -16, 12, 12, 17, 20, -16, -19, -18},
            {0, -12, 12, 12, 28, 25, -28, -17, -16},
            {0, -18, 12, 18, 18, 18, -20, -18, -17}
        };
        // y offsets in pixels: same layout as the x table
        constexpr int kOffsetY[kBodyTypes][9] = {
            {0, 60, 60, 60, 66, 66, 65, 60, 60},
            {0, 60, 60, 60, 66, 60, 65, 60, 60},
            {0, 60, 60, 60, 60, 60, 60, 60, 60},
            {0, 60, 60, 60, 60, 64, 60, 60, 60},
            {0, 60, 60, 60, 62, 65, 60, 60, 60}
        };

    }

    core::Point FishingRodOffsets::offset(int bodyType, int slot) {
        const int index = slot - kFirstSlot;
        if (bodyType < 0 || bodyType >= kBodyTypes || index < 0 || index > 8) { // last table index (slot 53)
            return {0, 0};
        }
        return {kOffsetX[bodyType][index], kOffsetY[bodyType][index]};
    }

}
