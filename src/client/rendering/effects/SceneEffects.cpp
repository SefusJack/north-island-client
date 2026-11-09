#include "client/rendering/effects/SceneEffects.h"

#include "client/rules/SceneEffectSelection.h"

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game view width, px
        constexpr int kScreenHeight = 600; // game view height, px

    }

    SceneEffects::SceneEffects(
        gfx::PictureLibrary& pictures
    )
        : m_snow(pictures) {
        for (const SceneEffectKind kind :
             {SceneEffectKind::Leaves,
              SceneEffectKind::FallingLeaves,
              SceneEffectKind::Rain,
              SceneEffectKind::Steam,
              SceneEffectKind::Bubbles,
              SceneEffectKind::Ribbons,
              SceneEffectKind::Stars}) {
            m_effects.emplace_back(kind, pictures);
        }
    }

    void SceneEffects::drawUnder(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs, const Conditions& conditions) {
        const SceneEffectSelection picked = SceneEffectSelection::of(conditions);
        if (picked.bubbles) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Bubbles).update(view, nowMs);
            }
        } else {
            effect(SceneEffectKind::Bubbles).clear();
        }
        if (picked.leaves) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Leaves).update(view, nowMs);
            }
            effect(SceneEffectKind::Leaves).drawUnder(canvas, view, nowMs);
        } else {
            effect(SceneEffectKind::Leaves).clear();
        }
        if (picked.fallingLeaves) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::FallingLeaves).update(view, nowMs);
            }
            effect(SceneEffectKind::FallingLeaves).drawUnder(canvas, view, nowMs);
        } else {
            effect(SceneEffectKind::FallingLeaves).clear();
        }
        if (picked.rain) {
            canvas.fillRect({0, 0, kScreenWidth, kScreenHeight}, {40, 40, 40, 200}); // dark grey veil (r, g, b, alpha 200) that darkens the map in rain
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Rain).update(view, nowMs);
            }
            effect(SceneEffectKind::Rain).drawUnder(canvas, view, nowMs);
        } else {
            effect(SceneEffectKind::Rain).clear();
        }
        if (picked.stars) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Stars).update(view, nowMs);
            }
            effect(SceneEffectKind::Stars).drawUnder(canvas, view, nowMs);
        } else {
            effect(SceneEffectKind::Stars).clear();
        }
        if (picked.ribbons && conditions.wedding) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Ribbons).update(view, nowMs);
            }
            effect(SceneEffectKind::Ribbons).drawUnder(canvas, view, nowMs);
        } else {
            effect(SceneEffectKind::Ribbons).clear();
        }
    }

    void SceneEffects::drawOver(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs, const Conditions& conditions) {
        const SceneEffectSelection picked = SceneEffectSelection::of(conditions);
        if (picked.leaves) {
            effect(SceneEffectKind::Leaves).drawOver(canvas, view, nowMs);
        }
        if (picked.fallingLeaves) {
            effect(SceneEffectKind::FallingLeaves).drawOver(canvas, view, nowMs);
        }
        if (picked.bubbles) {
            effect(SceneEffectKind::Bubbles).drawOver(canvas, view, nowMs);
        }
        if (picked.ribbons) {
            if (!conditions.wedding) {
                return;
            }
            effect(SceneEffectKind::Ribbons).drawOver(canvas, view, nowMs);
        }
        if (picked.rain) {
            constexpr int kFlashChance = 10; // 10 in 400 per animation frame that a lightning flash shows
            effect(SceneEffectKind::Rain).drawOver(canvas, view, nowMs);
            if (canvas.isAnimationFrame()) {
                m_flash = std::uniform_int_distribution<int>(0, 399)(m_random) < kFlashChance; // 399: a roll of 0..399 (the chance is out of 400)
            }
            if (m_flash) {
                canvas.fillRect({0, 0, kScreenWidth, kScreenHeight}, {200, 200, 200, 200}); // light grey flash (r, g, b, alpha 200) of the lightning
            }
        }
        if (picked.snow) {
            if (canvas.isAnimationFrame()) {
                m_snow.update(view, nowMs);
            }
            m_snow.draw(canvas, view);
        } else {
            m_snow.clear();
        }
        if (picked.steam) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Steam).update(view, nowMs);
            }
            effect(SceneEffectKind::Steam).drawOver(canvas, view, nowMs);
        } else {
            effect(SceneEffectKind::Steam).clear();
        }
        if (picked.stars) {
            if (canvas.isAnimationFrame()) {
                effect(SceneEffectKind::Stars).update(view, nowMs);
            }
            effect(SceneEffectKind::Stars).drawOver(canvas, view, nowMs);
        }
    }

}
