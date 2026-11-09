#include "client/rendering/effects/TransmitEffect.h"

namespace client {

    namespace {

        constexpr std::uint64_t kFirstDurationMs = 1500; // departure phase lasts 1.5 s
        constexpr std::uint64_t kGoneAfterMs = 900; // departing: character counts as gone 0.9 s after the start
        constexpr std::uint64_t kArrivalDurationMs = 6000; // arrival phase lasts 6 s
        constexpr std::uint64_t kBackAfterMs = 4800; // arriving: character is shown again after 4.8 s
        constexpr std::uint64_t kFrameMs = 180; // light frame time, ms
        constexpr int kLightAlpha = 24; // light blend strength (0..255)
        constexpr int kLightBelowFeet = 50; // while running, the light follows 50 px below the character's feet
        constexpr int kLightStartAboveFeet = 5; // the light starts 5 px above the character's feet

    }

    void TransmitEffect::begin(int place, core::Point feet, std::uint64_t nowMs) {
        m_place = place;
        m_arrival = false;
        start(feet, nowMs);
    }

    void TransmitEffect::start(core::Point feet, std::uint64_t nowMs) {
        m_on = true;
        if (!m_running) {
            m_running = true;
            m_tickMs = nowMs;
            m_durationMs = kFirstDurationMs;
        }
        const int frames = m_arrival ? 17 : 15; // frames of S10123 (arrival) and S10108 (leaving)
        if (const auto picture = m_pictures.find(m_arrival ? "S10123" : "S10108")) {
            LightEffect::Setup setup;
            setup.picture = *picture;
            setup.frameMs = kFrameMs;
            setup.firstFrame = 1;
            setup.lastFrame = frames;
            setup.frameCount = frames;
            setup.motion = LightEffect::Motion::Once;
            setup.loop = false;
            setup.alpha = kLightAlpha;
            m_light.start(setup, {feet.x, feet.y - kLightStartAboveFeet}, nowMs);
        }
    }

    void TransmitEffect::arrive(core::Point feet, std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        start(feet, nowMs);
        m_arrival = true;
    }

    bool TransmitEffect::update(core::Point feet, std::uint64_t nowMs) {
        if (!m_running) {
            return false;
        }
        m_light.setPosition({feet.x, feet.y + kLightBelowFeet});
        const std::uint64_t elapsed = nowMs - m_tickMs;
        if (m_arrival) {
            m_durationMs = kArrivalDurationMs;
            m_on = elapsed < kBackAfterMs;
        } else {
            m_on = elapsed >= kGoneAfterMs;
        }
        if (elapsed < m_durationMs) {
            return false;
        }
        if (m_on) {
            if (!m_arrival) {
                m_arrival = true;
                start(feet, nowMs);
                return true;
            }
            return false;
        }
        m_arrival = false;
        m_running = false;
        m_light.stop();
        return false;
    }

    void TransmitEffect::draw(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (m_running && m_light.isActive()) {
            m_light.draw(canvas, nowMs);
        }
    }

}
