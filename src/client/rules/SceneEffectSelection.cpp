#include "client/rules/SceneEffectSelection.h"

namespace client {

    namespace {

        // values of the map's scene effect field (conditions.sceneEffect)
        constexpr int kLeavesScene = 1;
        constexpr int kSnowScene = 2;
        constexpr int kSteamScene = 3;
        constexpr int kRainScene = 4;
        constexpr int kFallingLeavesScene = 5;
        constexpr int kBubblesScene = 6;
        constexpr int kRibbonsScene = 7;
        constexpr int kStarMap = 10993; // map id that always shows the stars effect

        // effect kinds a forced effect can name (conditions.forceKind; 0 = none forced)
        constexpr int kLeavesKind = 1; // leaves
        constexpr int kSnowKind = 2; // snow
        constexpr int kRainKind = 3; // rain
        constexpr int kStarsKind = 5; // stars
        constexpr int kSteamKind = 10; // steam
        constexpr int kFallingLeavesKind = 11; // falling leaves
        constexpr int kBubblesKind = 12; // bubbles
        constexpr int kRibbonsKind = 13; // ribbons

    }

    SceneEffectSelection SceneEffectSelection::of(const SceneEffectConditions& conditions) {
        const auto picked = [&conditions](bool mapHas, int kind) { return (mapHas && conditions.forceKind == 0) || conditions.forceKind == kind; };
        SceneEffectSelection selection;
        selection.leaves = picked(conditions.sceneEffect == kLeavesScene, kLeavesKind);
        selection.snow = picked(conditions.sceneEffect == kSnowScene, kSnowKind) || conditions.christmas;
        selection.rain = picked(conditions.sceneEffect == kRainScene, kRainKind);
        selection.stars = picked(conditions.mapId == kStarMap, kStarsKind);
        selection.steam = picked(conditions.sceneEffect == kSteamScene, kSteamKind);
        selection.fallingLeaves = picked(conditions.sceneEffect == kFallingLeavesScene, kFallingLeavesKind);
        selection.bubbles = picked(conditions.sceneEffect == kBubblesScene, kBubblesKind);
        selection.ribbons = picked(conditions.sceneEffect == kRibbonsScene, kRibbonsKind);
        return selection;
    }

}
