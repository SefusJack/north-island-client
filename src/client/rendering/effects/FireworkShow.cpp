#include "client/rendering/effects/FireworkShow.h"

#include <optional>
#include <string>

namespace client {

    namespace {

        constexpr std::uint64_t kLimitMs = 2800; // the whole show is cut off after 2.8 s
        constexpr std::uint64_t kRocketFrameMs = 280; // rocket frame time, ms
        constexpr int kRocketFrames = 6; // frames in the rocket pictures (S51016L, S50616L)
        constexpr double kRisePerMs = 0.1; // rocket climbs 0.1 px per ms (100 px/s)
        constexpr int kLaunchRise = 50; // rocket starts 50 px above the launch point
        constexpr int kBurstFrames = 7; // default frame count of a burst picture

    }

    void FireworkShow::setup(
        LightEffect& light,
        const char* name,
        std::uint64_t frameMs,
        int frames,
        int offsetY,
        std::uint64_t nowMs,
        core::Point at
    ) {
        const auto picture = m_pictures.find(name);
        if (!picture) {
            light.stop();
            return;
        }
        constexpr int kLightAlpha = 30; // light blend strength (0..255)
        LightEffect::Setup set;
        set.picture = *picture;
        set.frameMs = frameMs;
        set.firstFrame = 1;
        set.lastFrame = frames;
        set.frameCount = frames;
        set.motion = LightEffect::Motion::Once;
        set.loop = false;
        set.alpha = kLightAlpha;
        set.offsetY = offsetY;
        light.start(set, at, nowMs);
    }

    void FireworkShow::start(int kind, int variant, core::Point at, std::uint64_t nowMs) {
        m_kind = kind;
        m_variant = variant;
        m_running = true;
        m_bursting = false;
        m_rise = 0;
        // kind 4 (two bursts): the second burst starts when the first reaches its 3rd frame; else on the 1st
        m_secondStart = kind == 4 ? 3 : 1;
        m_anchor = {at.x, at.y - kLaunchRise};
        m_startMs = nowMs;
        m_lastMs = nowMs;
        m_burst.stop();
        m_second.stop();
        constexpr int kLightOffset = 25; // rocket pictures are drawn 25 px down from their point
        setup(m_rocket, "s51016L", kRocketFrameMs, kRocketFrames, kLightOffset, nowMs, m_anchor);
        setup(m_rocketGlow, "S50616L", kRocketFrameMs, kRocketFrames, kLightOffset, nowMs, m_anchor);
        m_playSound("sound/SEB0077.wav"); // launch sound
    }

    bool FireworkShow::draw(gfx::Canvas& canvas, core::Point origin, std::uint64_t nowMs) {
        if (!m_running) {
            return false;
        }
        if (nowMs - m_startMs > kLimitMs) {
            m_running = false;
            return false;
        }
        if (!m_bursting) {
            m_rise += static_cast<double>(nowMs - m_lastMs) * kRisePerMs;
            const core::Point at{m_anchor.x, m_anchor.y - static_cast<int>(m_rise)};
            m_rocket.setPosition(at);
            m_rocketGlow.setPosition({at.x, at.y + 2}); // the glow sits 2 pixels below the rocket
            m_rocket.draw(canvas, nowMs, origin);
            m_rocketGlow.draw(canvas, nowMs, origin);
            if (!m_rocket.isActive()) {
                m_bursting = true;
                m_anchor = at;
                std::uint64_t frameMs = 90; // burst frame time in ms
                int frames = kBurstFrames;
                std::optional<std::string> name;
                m_burstOffset = 0;
                switch (m_kind) {
                    case 1: // kind 1: burst picture by variant (2 S40002, 3 S40003, else S40004)
                        name = m_variant == 2 ? "S40002" : m_variant == 3 ? "S40003" : "S40004";
                        break;
                    case 2: // kind 2: burst S40001
                    case 4: // kind 4: burst S40001 plus a second burst S40013
                        name = "S40001";
                        m_burstOffset = 100; // burst is drawn 100 pixels below where the rocket ended
                        break;
                    case 3: // kind 3: burst S40010, 5 frames of 150 ms
                        name = "S40010";
                        frameMs = 150; // burst frame time in ms
                        frames = 5; // burst frame count
                        m_burstOffset = 80; // burst is drawn 80 pixels below where the rocket ended
                        break;
                    default: // unknown kind: no burst
                        break;
                }
                if (name) {
                    // 25: pixels the burst is drawn down from its point
                    setup(m_burst, name->c_str(), frameMs, frames, 25, nowMs, {at.x, at.y + m_burstOffset});
                }
                if (m_kind == 4) {
                    // second burst: frame time 126 ms, 6 frames, 25 pixels down, placed 150 pixels above the first
                    setup(m_second, "S40013", 126, 6, 25, nowMs, {at.x, at.y + m_burstOffset - 150});
                }
                m_playSound("sound/SEB0051.wav"); // burst sound
            }
        } else {
            m_burst.draw(canvas, nowMs, origin);
            if (m_second.isActive() && m_burst.frameIndex() + 1 >= m_secondStart) {
                m_second.draw(canvas, nowMs, origin);
            }
            if (!m_burst.isActive() && !m_second.isActive()) {
                m_running = false;
            }
        }
        m_lastMs = nowMs;
        return m_running;
    }

}
