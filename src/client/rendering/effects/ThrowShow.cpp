#include "client/rendering/effects/ThrowShow.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr double kSpeed = 0.36; // flight speed, px per ms
        constexpr int kLaunchRise = 50; // the throw starts 50 px above the thrower's point
        constexpr int kImpactAlpha = 25; // impact light blend strength (0..255)
        constexpr std::size_t kMostThrows = 40; // at most 40 throws in the air at once

        /// @brief The look of one kind of throw: its flight picture, its impact strip and the impact sound.
        /// @details Original: HL_NewFireworks.pas flight pictures (one frame each) and impacts by kind.
        struct ImpactKind {
            const char* flight;
            const char* picture;
            int firstFrame;
            int lastFrame;
            int frameCount;
            std::uint64_t frameMs;
            int offsetY;
            const char* sound;
        };

        // per kind 1..3: flight picture, impact picture, first frame, last frame, frames in the picture, frame time in ms, impact offset down in pixels, impact sound
        constexpr ImpactKind kKinds[] = {
            {"3369", "S10797", 1, 10, 10, 10, 30, "sound/SEB0153.wav"},
            {"3431", "S10241", 8, 12, 19, 10, 65, "sound/SEB0031.wav"},
            {"3440", "S10767", 1, 13, 13, 50, 85, "sound/SEB0017.wav"},
        };

    }

    void ThrowShow::start(int kind, core::Point from, core::Point to, std::uint64_t nowMs) {
        if (kind < 1 || kind > 3 || m_throws.size() >= kMostThrows) { // 3: the throw kinds are 1..3
            return;
        }
        const ImpactKind& look = kKinds[kind - 1];
        const auto picture = m_pictures.find(look.flight);
        if (!picture) {
            return;
        }
        Throw thrown;
        thrown.kind = kind;
        thrown.target = to;
        LightEffect::Setup setup;
        setup.picture = *picture;
        setup.frameMs = 10; // flight frame time in ms
        setup.motion = LightEffect::Motion::Arc;
        setup.target = to;
        setup.speed = kSpeed;
        setup.offsetY = 10; // flight picture is drawn 10 pixels down from the point
        thrown.flight.start(setup, {from.x, from.y - kLaunchRise}, nowMs);
        m_throws.push_back(std::move(thrown));
        m_playSound("sound/wav1608.wav"); // throw sound
    }

    void ThrowShow::draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) {
        for (Throw& thrown : m_throws) {
            if (!thrown.landed) {
                thrown.flight.update(nowMs);
                if (thrown.flight.isActive()) {
                    thrown.flight.draw(canvas, nowMs, origin);
                }
                if (!thrown.flight.isActive() || thrown.flight.atTarget()) {
                    thrown.landed = true;
                    const ImpactKind& look = kKinds[thrown.kind - 1];
                    if (const auto picture = m_pictures.find(look.picture)) {
                        LightEffect::Setup setup;
                        setup.picture = *picture;
                        setup.frameMs = look.frameMs;
                        setup.firstFrame = look.firstFrame;
                        setup.lastFrame = look.lastFrame;
                        setup.frameCount = look.frameCount;
                        setup.motion = LightEffect::Motion::Once;
                        setup.loop = false;
                        setup.alpha = kImpactAlpha;
                        setup.offsetY = look.offsetY;
                        thrown.impact.start(setup, thrown.target, nowMs);
                    }
                    m_playSound(look.sound);
                    m_landed(thrown.kind, thrown.target);
                }
            } else {
                thrown.impact.draw(canvas, nowMs, origin);
            }
        }
        m_throws.erase(
            std::remove_if(m_throws.begin(), m_throws.end(), [](const Throw& thrown) { return thrown.landed && !thrown.impact.isActive(); }),
            m_throws.end()
        );
    }

}
