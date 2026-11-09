#ifndef NORTH_ISLAND_CLIENT_RULES_SCENE_EFFECT_SELECTION_H
#define NORTH_ISLAND_CLIENT_RULES_SCENE_EFFECT_SELECTION_H

#include "client/rules/SceneEffectConditions.h"

namespace client {

    /// @brief Which effects the tests of DrawSceneEffectsUnder / DrawSceneEffectsOver let through.
    /// @details Original: `(HasSceneEffect(kind, MapId) and (AForceKind = 0)) or (AForceKind = kind)` for each kind (the snow also whenever the Christmas event is on a Christmas map).
    struct SceneEffectSelection {
        bool leaves = false;
        bool snow = false;
        bool rain = false;
        bool stars = false;
        bool steam = false;
        bool fallingLeaves = false;
        bool bubbles = false;
        bool ribbons = false;

        /// @brief Works out the selected effects.
        /// @param conditions The map, scene effect, event flags and force kind.
        /// @return The selection.
        static SceneEffectSelection of(const SceneEffectConditions& conditions);
    };

}

#endif
