#include "client/rendering/effects/GpsTeleportEffect.h"

namespace client {

    namespace {

        constexpr std::uint64_t kFirstDurationMs = 1500; // departure phase lasts 1.5 s
        constexpr std::uint64_t kGoneAfterMs = 900; // departing: character counts as gone 0.9 s after the start
        constexpr std::uint64_t kArrivalDurationMs = 6000; // arrival phase lasts 6 s
        constexpr std::uint64_t kBackAfterMs = 4800; // arriving: character is shown again after 4.8 s
        constexpr std::uint64_t kFrameMs = 200; // light frame time, ms
        constexpr int kFrames = 15; // frames in picture S10882
        constexpr int kLightAlpha = 8; // light blend strength (0..255)
        constexpr int kLightOffsetY = 30; // picture is drawn 30 px down from its point
        constexpr int kLightAboveFeet = 15; // light is placed 15 px above the character's feet

    }

    void GpsTeleportEffect::start(core::Point feet, std::uint64_t nowMs) {
        m_on = true;
        if (!m_running) {
            m_running = true;
            m_tickMs = nowMs;
            m_durationMs = kFirstDurationMs;
        }
        if (const auto picture = m_pictures.find("S10882")) {
            LightEffect::Setup setup;
            setup.picture = *picture;
            setup.frameMs = kFrameMs;
            setup.firstFrame = 1;
            setup.lastFrame = kFrames;
            setup.frameCount = kFrames;
            setup.motion = LightEffect::Motion::Stay;
            setup.loop = true;
            setup.alpha = kLightAlpha;
            setup.offsetY = kLightOffsetY;
            m_light.start(setup, feet, nowMs);
        }
    }

    void GpsTeleportEffect::begin(core::Point feet, std::uint64_t nowMs) {
        m_arrival = false;
        start(feet, nowMs);
    }

    void GpsTeleportEffect::arrive(core::Point feet, std::uint64_t nowMs) {
        if (!m_on) {
            return;
        }
        m_arrival = true;
        start(feet, nowMs);
    }

    bool GpsTeleportEffect::update(core::Point feet, std::uint64_t nowMs) {
        if (!m_running) {
            return false;
        }
        m_light.setPosition({feet.x, feet.y - kLightAboveFeet});
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

    void GpsTeleportEffect::draw(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (m_running && m_light.isActive()) {
            m_light.draw(canvas, nowMs);
        }
    }

}
