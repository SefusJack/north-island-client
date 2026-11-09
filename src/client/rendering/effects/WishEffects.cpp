#include "client/rendering/effects/WishEffects.h"

#include <array>

namespace client {

    namespace {

        constexpr std::uint64_t kBalloonMs = 6000; // a blessing balloon lasts 6 s
        constexpr std::uint64_t kFrameMs = 150; // light frame time, ms
        constexpr std::uint64_t kHeartsMs = 10000; // the hearts last 10 s
        constexpr int kBalloonFrames = 4; // frames in picture S40015
        constexpr int kHeartsFrames = 9; // frames in picture S40011
        constexpr int kHeartsAlpha = 30; // light blend strength (0..255)
        constexpr int kHeartsOffsetY = 25; // hearts are drawn 25 px down from their point
        constexpr int kRise = 3; // balloon rises 3 px per drawn frame
        constexpr int kSide = 50; // the balloon starts 50 px away from the player, toward the player's facing

        // 8: one start offset per direction 0 to 7
        constexpr std::array<core::Point, 8> kStart{
            {{0, -kSide}, {-kSide, -kSide}, {-kSide, 0}, {-kSide, kSide}, {0, kSide}, {kSide, kSide}, {kSide, 0}, {kSide, -kSide}}
        };

    }

    void WishEffects::startBless(std::int32_t playerId, core::Point feet, std::optional<int> direction, std::uint64_t nowMs) {
        Balloon& balloon = m_balloons[playerId];
        if (!balloon.light.isActive()) {
            if (const auto picture = m_pictures.find("S40015")) {
                LightEffect::Setup setup;
                setup.picture = *picture;
                setup.frameMs = kFrameMs;
                setup.firstFrame = 1;
                setup.lastFrame = kBalloonFrames;
                setup.frameCount = kBalloonFrames;
                setup.motion = LightEffect::Motion::Stay;
                setup.loop = true;
                balloon.light.start(setup, {1, 1}, nowMs);
            }
        }
        // 8: the directions 0 to 7
        const core::Point side = direction && *direction >= 0 && *direction < 8 ? kStart[static_cast<std::size_t>(*direction)] : core::Point{-kSide, 0};
        balloon.at = {feet.x + side.x, feet.y + side.y};
        balloon.startMs = nowMs;
        m_playSound("sound/9911.wav"); // blessing sound
        m_playSound("sound/SEB0088.wav"); // second blessing sound
    }

    void WishEffects::startHeartsLight(Hearts& hearts, std::uint64_t nowMs) {
        if (const auto picture = m_pictures.find("S40011")) {
            LightEffect::Setup setup;
            setup.picture = *picture;
            setup.frameMs = kFrameMs;
            setup.firstFrame = 1;
            setup.lastFrame = kHeartsFrames;
            setup.frameCount = kHeartsFrames;
            setup.motion = LightEffect::Motion::Once;
            setup.loop = false;
            setup.alpha = kHeartsAlpha;
            setup.offsetY = kHeartsOffsetY;
            hearts.light.start(setup, {1, 1}, nowMs);
        }
    }

    void WishEffects::startHearts(std::int32_t playerId, std::uint64_t nowMs) {
        Hearts& hearts = m_hearts[playerId];
        hearts.startMs = nowMs;
        startHeartsLight(hearts, nowMs);
    }

    bool WishEffects::draw(
        gfx::Canvas& canvas,
        core::Point origin,
        std::uint64_t nowMs,
        const std::function<std::optional<core::Point>(std::int32_t)>& positionOf
    ) {
        bool ended = false;
        for (auto it = m_balloons.begin(); it != m_balloons.end();) {
            Balloon& balloon = it->second;
            if (nowMs - balloon.startMs > kBalloonMs) {
                ended = true;
                it = m_balloons.erase(it);
                continue;
            }
            balloon.at.y -= kRise;
            balloon.light.setPosition({balloon.at.x - origin.x, balloon.at.y - origin.y});
            balloon.light.draw(canvas, nowMs);
            ++it;
        }
        for (auto it = m_hearts.begin(); it != m_hearts.end();) {
            Hearts& hearts = it->second;
            if (nowMs - hearts.startMs > kHeartsMs) {
                it = m_hearts.erase(it);
                continue;
            }
            if (!hearts.light.isActive()) {
                startHeartsLight(hearts, nowMs);
            }
            if (const auto where = positionOf(it->first)) {
                hearts.light.setPosition({where->x - origin.x, where->y - origin.y});
                hearts.light.draw(canvas, nowMs);
            }
            ++it;
        }
        return ended;
    }

}
