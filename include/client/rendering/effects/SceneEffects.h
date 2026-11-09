#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SCENE_EFFECTS_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SCENE_EFFECTS_H

#include "client/rendering/effects/SceneEffect.h"
#include "client/rules/SceneEffectConditions.h"
#include "client/rendering/effects/SnowEffect.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <random>
#include <vector>

namespace client {

    /// @brief All the effects of a scene and the order they are drawn in.
    /// @details Original: TFT_EffectManage with DrawSceneEffectsUnder / DrawSceneEffectsOver. Which effects the map has (HasSceneEffect) or a
    /// movie forces (AForceKind), some drawn under the people and the rest over them.
    class SceneEffects {
        public:
            using Conditions = SceneEffectConditions;

            /// @brief Creates the effects.
            /// @param pictures Picture library the effects' pictures are looked up in.
            explicit SceneEffects(
                gfx::PictureLibrary& pictures
            );

            /// @brief Draws the effects that go under the people.
            /// @details Original: DrawSceneEffectsUnder.
            /// @param canvas Canvas to draw on.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            /// @param conditions Which effects the map has or a movie forces.
            void drawUnder(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs, const Conditions& conditions);

            /// @brief Draws the effects that go over the people.
            /// @details Original: DrawSceneEffectsOver.
            /// @param canvas Canvas to draw on.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            /// @param conditions Which effects the map has or a movie forces.
            void drawOver(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs, const Conditions& conditions);

        private:
            /// @brief The effect object of a kind.
            /// @param kind Effect kind.
            /// @return The effect.
            SceneEffect& effect(SceneEffectKind kind) { return m_effects[static_cast<std::size_t>(kind)]; }

            std::vector<SceneEffect> m_effects;
            SnowEffect m_snow;
            std::mt19937 m_random{std::random_device{}()};
            bool m_flash = false;
    };

}

#endif
