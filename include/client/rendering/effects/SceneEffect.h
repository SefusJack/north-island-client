#ifndef NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SCENE_EFFECT_H
#define NORTH_ISLAND_CLIENT_RENDERING_EFFECTS_SCENE_EFFECT_H

#include "client/rules/SceneEffectKind.h"
#include "client/rendering/effects/SceneParticle.h"
#include "core/Point.h"
#include "core/Size.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

namespace client {

    /// @brief One scene effect (leaves, rain, steam, bubbles, ribbons or stars) made of falling or rising particles.
    /// @details Original: FT_SceneEffect with UpdateLeafEffect, UpdateRainEffect, UpdateSteamEffect, UpdateBubbleEffect, UpdateRibbonEffect,
    /// UpdateStarEffect and their Reset / Move / Draw routines. Particles start at the screen's edge over the map, one or a few at a time,
    /// each moving by the time since its last frame change and then gone. Falling ones draw over the people, those that have landed
    /// (the rain's splashes) under them.
    class SceneEffect {
        public:
            /// @brief Creates the effect and loads its pictures.
            /// @param kind Which effect it is.
            /// @param pictures Picture library the effect's pictures are looked up in.
            SceneEffect(
                SceneEffectKind kind,
                gfx::PictureLibrary& pictures
            );

            /// @brief Starts new particles when due, moves every one and drops the landed ones.
            /// @details Original: Update*Effect.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void update(core::Point view, std::uint64_t nowMs);

            /// @brief Draws the particles that have landed (under the people).
            /// @details Original: DrawLeavesUnder, DrawRainUnder, DrawStarsUnder.
            /// @param canvas Canvas to draw on.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void drawUnder(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs);

            /// @brief Draws the particles still on their way (over the people).
            /// @details Original: DrawLeavesOver, DrawRainDrop, DrawSteam, DrawBubbles, DrawRibbons, DrawStarsOver.
            /// @param canvas Canvas to draw on.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void drawOver(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs);

            /// @brief Switches the effect off: the particles go (the next time it starts afresh).
            void clear();

        private:
            static constexpr std::size_t kMostPictures = 6;

            /// @brief Draws a random number.
            /// @param count Number of values.
            /// @return A value in 0..count-1.
            int random(int count);

            /// @brief Gives a particle its start place, target and look.
            /// @details Original: Reset* routines; over the map points the screen shows.
            /// @param particle Particle to start.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void spawn(SceneParticle& particle, core::Point view, std::uint64_t nowMs);

            /// @brief Moves a particle towards its target by the time since its last frame change and steps its frames.
            /// @details Original: Move* routines.
            /// @param particle Particle to move.
            /// @param nowMs Current tick in milliseconds.
            void move(SceneParticle& particle, std::uint64_t nowMs);

            /// @brief Advances the sideways sway of a particle.
            /// @details Original: SwayLeaf, AnimateRibbon, AnimateSteam.
            /// @param particle Particle to sway.
            /// @param nowMs Current tick in milliseconds.
            void sway(SceneParticle& particle, std::uint64_t nowMs);

            /// @brief Draws a frame of a particle's picture (steam and stars add their light).
            /// @details Original: Draw* routines.
            /// @param canvas Canvas to draw on.
            /// @param particle Particle to draw.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            void draw(gfx::Canvas& canvas, SceneParticle& particle, core::Point view, std::uint64_t nowMs);

            /// @brief Draws the particles a test picks, as draw does each, but an additive kind (steam, stars) grouped by picture.
            /// @details Added light is the same in any order, and draws of one texture in a row merge into one GPU draw: steam
            /// drawn in spawn order switched among its pictures nearly every particle, thousands of draws a frame on a large
            /// window (the set is drawn once per 800x600 tile). The sway still runs in spawn order first.
            /// @param canvas Canvas to draw on.
            /// @param view Map point at the screen's top-left corner.
            /// @param nowMs Current tick in milliseconds.
            /// @param shown Whether a particle is drawn.
            void drawParticles(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs, const std::function<bool(const SceneParticle&)>& shown);

            /// @brief The speed of a particle for its speed level.
            /// @param particle Particle asked about.
            /// @return Pixels per millisecond.
            double speed(const SceneParticle& particle) const;

            /// @brief Tells whether this kind of effect sways sideways.
            /// @return True for leaves, ribbons and steam.
            bool swaying() const;

            SceneEffectKind m_kind;
            std::array<gfx::PictureId, kMostPictures> m_pictures;
            std::array<core::Size, kMostPictures> m_frameSizes;
            std::vector<SceneParticle> m_particles;
            std::uint64_t m_spawnMs = 0;
            std::mt19937 m_random{std::random_device{}()};
    };

}

#endif
