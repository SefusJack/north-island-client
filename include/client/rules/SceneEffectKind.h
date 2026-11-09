#ifndef NORTH_ISLAND_CLIENT_RULES_SCENE_EFFECT_KIND_H
#define NORTH_ISLAND_CLIENT_RULES_SCENE_EFFECT_KIND_H

namespace client {

    /// @brief The scene effects of FT_SceneEffect besides the snow (HasSceneEffect's kinds 1, 11, 3, 10, 12, 13 and 5).
    enum class SceneEffectKind {
        Leaves,
        FallingLeaves,
        Rain,
        Steam,
        Bubbles,
        Ribbons,
        Stars,
    };

}

#endif
