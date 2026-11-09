#ifndef NORTH_ISLAND_CLIENT_RULES_SCENE_EFFECT_CONDITIONS_H
#define NORTH_ISLAND_CLIENT_RULES_SCENE_EFFECT_CONDITIONS_H

namespace client {

    /// @brief What decides the effects DrawSceneEffectsUnder / DrawSceneEffectsOver draw: the map the player is on (HasSceneEffect) and AForceKind, the effect a story movie asks for in its place.
    struct SceneEffectConditions {
        int mapId = 0;
        int sceneEffect = 0;
        bool christmas = false;
        bool wedding = false;
        int forceKind = 0;
    };

}

#endif
