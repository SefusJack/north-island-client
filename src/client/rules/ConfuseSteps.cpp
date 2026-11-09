#include "client/rules/ConfuseSteps.h"

namespace client {

    namespace {

        constexpr int kFirstStone = 61063; // item 61063: Morning Star Crystal, the first confuse stone (step 1)
        constexpr int kSteps = 16; // 16 consecutive stone items (61063..61078), steps 1..16
        constexpr int kChaosCrystal = 61062; // item 61062: Chaos Crystal
        constexpr int kShadowChaosCrystal = 61079; // item 61079: Shadow Chaos Crystal
        constexpr int kPureChaosCrystal = 61080; // item 61080: Pure Chaos Crystal

    }

    int confuseStoneStep(int stoneItemId) { return stoneItemId >= kFirstStone && stoneItemId < kFirstStone + kSteps ? stoneItemId - kFirstStone + 1 : 0; }

    int confuseCrystalLevel(int itemId) {
        switch (itemId) {
            case kChaosCrystal: return 1; // Chaos Crystal 61062: level 1
            case kShadowChaosCrystal: return 2; // Shadow Chaos Crystal 61079: level 2
            case kPureChaosCrystal: return 3; // Pure Chaos Crystal 61080: level 3
            default: return 0; // any other item: no level
        }
    }

}
