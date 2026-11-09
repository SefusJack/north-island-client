#include "client/rendering/effects/TimedLight.h"

namespace client {

    namespace {

        constexpr std::uint64_t kMinuteMs = 60000; // each start adds one minute of light
        constexpr std::uint64_t kFrameMs = 60; // light frame time, ms
        constexpr int kFrames = 16; // frames in picture S10233
        constexpr int kAlpha = 30; // light blend strength (0..255)
        constexpr int kOffsetY = 25; // picture is drawn 25 px down from its point
        constexpr int kAboveFeet = 5; // light is placed 5 px above the character's feet

    }

    void TimedLight::start(core::Point feet, std::uint64_t nowMs) {
        if (m_on) {
            m_durationMs += kMinuteMs;
        } else {
            m_on = true;
            m_startMs = nowMs;
            m_durationMs = kMinuteMs;
        }
        play(feet, nowMs);
    }

    void TimedLight::play(core::Point feet, std::uint64_t nowMs) {
        const auto picture = m_pictures.find("S10233");
        if (!picture) {
            return;
        }
        LightEffect::Setup setup;
        setup.picture = *picture;
        setup.frameMs = kFrameMs;
        setup.firstFrame = 1;
        setup.lastFrame = kFrames;
        setup.frameCount = kFrames;
        setup.motion = LightEffect::Motion::Once;
        setup.alpha = kAlpha;
        setup.offsetY = kOffsetY;
        m_light.start(setup, {feet.x, feet.y - kAboveFeet}, nowMs);
    }

    void TimedLight::draw(gfx::Canvas& canvas, core::Point feet, std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        m_light.setPosition({feet.x, feet.y - kAboveFeet});
        m_light.draw(canvas, nowMs);
        if (!m_light.isActive()) {
            play(feet, nowMs);
        }
        if (nowMs - m_startMs >= m_durationMs) {
            m_on = false;
            m_light.stop();
        }
    }

}
