#include "client/rendering/effects/SceneEffect.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game view width, px
        constexpr int kScreenHeight = 600; // game view height, px

        /// @brief Rounds like Delphi's Round: halves go to the even number.
        /// @param value Value to round.
        /// @return The rounded integer.
        int roundHalfEven(double value) { return static_cast<int>(std::nearbyint(value)); }

        /// @brief Names of the pictures an effect kind uses (indexed by a particle's picture number).
        /// @param kind Scene effect kind.
        /// @return The picture names, empty for an unknown kind.
        std::vector<std::string> pictureNames(SceneEffectKind kind) {
            switch (kind) {
                case SceneEffectKind::Leaves: return {"icon_leaves"}; // leaves: one picture
                case SceneEffectKind::FallingLeaves: return {"icon_leaves_2"}; // falling leaves: one picture
                case SceneEffectKind::Rain: return {"icon_Rains", "icon_Rains"}; // rain: the same picture for drop and splash
                case SceneEffectKind::Steam: return {"icon_steam_1", "icon_steam_2", "icon_steam_3", "icon_steam_4"}; // steam: four puff kinds
                case SceneEffectKind::Bubbles: return {"Icon_Bubble_1", "Icon_Bubble_2"}; // bubbles: two sizes
                case SceneEffectKind::Ribbons: // ribbons: six colours
                    return {"Icon_Ribbon1", "Icon_Ribbon2", "Icon_Ribbon3", "Icon_Ribbon4", "Icon_Ribbon5", "Icon_Ribbon6"};
                case SceneEffectKind::Stars: return {"icon_Star1", "icon_Star1"}; // stars: the same picture for both kinds
            }
            return {};
        }

        /// @brief How many frames each picture of an effect kind stacks vertically.
        /// @param kind Scene effect kind.
        /// @return The frame count.
        int stackedFrames(SceneEffectKind kind) {
            switch (kind) {
                case SceneEffectKind::Leaves: // leaves: 5 frames
                case SceneEffectKind::FallingLeaves: return 5; // falling leaves: 5 frames
                case SceneEffectKind::Rain: // rain: 4 frames
                case SceneEffectKind::Bubbles: return 4; // bubbles: 4 frames
                case SceneEffectKind::Steam: return 10; // steam: 10 frames
                case SceneEffectKind::Ribbons: return 6; // ribbons: 6 frames
                case SceneEffectKind::Stars: return 11; // stars: 11 frames
            }
            return 1;
        }

        /// @brief The most particles an effect kind has on screen at once.
        /// @param kind Scene effect kind.
        /// @return The particle capacity.
        std::size_t capacity(SceneEffectKind kind) {
            switch (kind) {
                case SceneEffectKind::Leaves: // leaves: 8 particles
                case SceneEffectKind::FallingLeaves: return 8; // falling leaves: 8 particles
                case SceneEffectKind::Bubbles: // bubbles: 10 particles
                case SceneEffectKind::Stars: return 10; // stars: 10 particles
                default: return 200; // rain, steam and ribbons: 200 particles
            }
        }

        /// @brief The time between two starts of particles of an effect kind.
        /// @param kind Scene effect kind.
        /// @return Interval in milliseconds.
        std::uint64_t spawnIntervalMs(SceneEffectKind kind) {
            switch (kind) {
                case SceneEffectKind::Leaves: // leaves: every 900 ms
                case SceneEffectKind::FallingLeaves: return 900; // falling leaves: every 900 ms
                case SceneEffectKind::Rain: return 30; // rain: every 30 ms
                case SceneEffectKind::Steam: // steam: every 200 ms
                case SceneEffectKind::Bubbles: return 200; // bubbles: every 200 ms
                case SceneEffectKind::Ribbons: return 500; // ribbons: every 500 ms
                case SceneEffectKind::Stars: return 2000; // stars: every 2000 ms
            }
            return 500; // default interval in ms
        }

        /// @brief How many particles start at once.
        /// @param kind Scene effect kind.
        /// @return Ten for rain, else one.
        int spawnsAtOnce(SceneEffectKind kind) { return kind == SceneEffectKind::Rain ? 10 : 1; } // rain starts 10 drops at once

        /// @brief How long a landed particle stays (the time since its last frame change).
        /// @param kind Scene effect kind.
        /// @return Linger time in milliseconds.
        std::uint64_t lingerMs(SceneEffectKind kind) {
            switch (kind) {
                case SceneEffectKind::Rain: return 450; // rain: the splash stays 450 ms
                case SceneEffectKind::Steam: // steam: stays 50 ms
                case SceneEffectKind::Bubbles: return 50; // bubbles: stay 50 ms
                default: return 0; // others are gone on landing
            }
        }

        /// @brief The frame after which the picture's frames start again.
        /// @param kind Scene effect kind.
        /// @return The last frame number of the cycle.
        int lastFrame(SceneEffectKind kind) {
            switch (kind) {
                case SceneEffectKind::Leaves: // leaves: frames 0..4
                case SceneEffectKind::FallingLeaves: return 4; // falling leaves: frames 0..4
                case SceneEffectKind::Steam: return 9; // steam: frames 0..9
                case SceneEffectKind::Stars: return 10; // stars: frames 0..10
                default: return 3; // others: frames 0..3
            }
        }

        /// @brief Leaves and ribbons drift to one side or the other.
        /// @param x Start x.
        /// @param pick Side pick: 0 drifts right, else left.
        /// @param steps Number of 50-pixel steps.
        /// @return The target x.
        int driftedTarget(int x, int pick, int steps) { return pick == 0 ? x + steps * 50 : x - steps * 50; } // 50: pixels per drift step

    }

    SceneEffect::SceneEffect(
        SceneEffectKind kind,
        gfx::PictureLibrary& pictures
    )
        : m_kind(kind) {
        const std::vector<std::string> names = pictureNames(kind);
        for (std::size_t i = 0; i < names.size(); ++i) {
            m_pictures[i] = pictures.find(names[i]).value_or(gfx::PictureId{});
        }
        const int stacked = stackedFrames(kind);
        for (std::size_t i = 0; i < names.size(); ++i) {
            const gfx::PictureId source = kind == SceneEffectKind::Steam || kind == SceneEffectKind::Bubbles ? m_pictures[i] : m_pictures[0];
            if (source.isValid()) {
                const gfx::Picture& picture = pictures.picture(source);
                m_frameSizes[i] = {picture.width(), picture.height() / stacked};
            }
        }
        m_particles.resize(capacity(kind));
    }

    int SceneEffect::random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

    void SceneEffect::clear() {
        for (SceneParticle& particle : m_particles) {
            particle.active = false;
        }
    }

    bool SceneEffect::swaying() const {
        return m_kind == SceneEffectKind::Leaves || m_kind == SceneEffectKind::FallingLeaves || m_kind == SceneEffectKind::Ribbons ||
               m_kind == SceneEffectKind::Steam;
    }

    double SceneEffect::speed(const SceneParticle& particle) const {
        const int level = particle.speedLevel;
        switch (m_kind) {
            case SceneEffectKind::Leaves: // leaves: speed by level
            case SceneEffectKind::FallingLeaves: // falling leaves: speed by level
            case SceneEffectKind::Ribbons: return level == 1 ? 0.01 : level == 3 ? 0.02 : 0.015; // pixels per ms for speed level 1 / 3 / 2
            case SceneEffectKind::Steam: return level == 1 ? 0.005 : level == 3 ? 0.009 : 0.007; // steam: pixels per ms for speed level 1 / 3 / 2
            case SceneEffectKind::Rain: return level == 1 ? 0.3 : level == 3 ? 0.5 : 0.4; // rain: pixels per ms for speed level 1 / 3 / 2
            case SceneEffectKind::Stars: return level == 1 ? 0.1 : level == 3 ? 0.2 : 0.15; // stars: pixels per ms for speed level 1 / 3 / 2
            case SceneEffectKind::Bubbles: return particle.picture == 1 ? 0.015 : 0.02; // bubbles: pixels per ms by picture (1 slower)
        }
        return 0.015; // default speed in pixels per ms
    }

    void SceneEffect::spawn(SceneParticle& particle, core::Point view, std::uint64_t nowMs) {
        particle = SceneParticle{};
        particle.tickMs = nowMs;
        particle.speedLevel = random(3) + 1; // speed level 1..3
        particle.frameDelay = random(3) * 100 + 200; // frame delay 200, 300 or 400 ms
        const int side = random(2);
        switch (m_kind) {
            case SceneEffectKind::Leaves: // leaves start along the top edge
            case SceneEffectKind::FallingLeaves: // falling leaves start along the top edge
                particle.position = {random(kScreenWidth) + view.x, view.y};
                // 5: up to 4 drift steps
                particle.target = {driftedTarget(particle.position.x, side, random(5)), view.y + kScreenHeight};
                particle.shakeRange = random(30); // sway range 0..29 pixels
                break;
            case SceneEffectKind::Rain: // rain drops start along the top edge and slant left
                particle.frameDelay = 150; // frame delay in ms
                particle.position = {
                    random(1200) + view.x,
                    view.y
                }; // 1200: width of the start band in pixels (wider than the screen so slanting drops cover it)
                particle.target = {particle.position.x - 300, random(450) + view.y + 150}; // lands 300 pixels left and 150..599 pixels below the view top
                particle.picture = random(100) >= 90 ? 1 : 0; // 10 percent chance of the second picture
                break;
            case SceneEffectKind::Stars: // stars fall from the top edge, slanting left
                particle.frameDelay = 50; // frame delay in ms
                particle.position = {random(1200) + view.x, view.y}; // 1200: width of the start band in pixels
                particle.target = {particle.position.x - 300, view.y + kScreenHeight}; // ends 300 pixels left of the start
                particle.picture = random(100) >= 70 ? 1 : 0; // 30 percent chance of the second picture
                break;
            case SceneEffectKind::Steam: // steam rises from the bottom edge
                particle.position = {random(kScreenWidth) + view.x, random(100) + view.y + 500}; // starts 500..599 pixels below the view top
                particle.picture = random(4); // one of the four puff pictures
                particle.target = {driftedTarget(particle.position.x, side, random(5)), view.y}; // 5: up to 4 drift steps
                particle.shakeRange = random(10); // sway range 0..9 pixels
                break;
            case SceneEffectKind::Bubbles: // bubbles rise from below the screen
                particle.frameDelay = random(3) * 10 + 200; // frame delay 200, 210 or 220 ms
                particle.position = {random(1000) + view.x, random(100) + view.y + 800}; // x within 1000 pixels, y 800..899 below the view top
                particle.target = {particle.position.x, view.y - random(300)}; // rises up to 299 pixels above the view top
                particle.picture = random(100) >= 60 ? 1 : 0; // 40 percent chance of the second picture
                break;
            case SceneEffectKind::Ribbons: // ribbons fall from the top edge
                particle.speedLevel = random(5) + 1; // speed level 1..5
                particle.position = {random(kScreenWidth) + view.x, view.y};
                particle.picture = random(5); // one of the first five ribbon pictures
                // 5: up to 4 drift steps
                particle.target = {driftedTarget(particle.position.x, side, random(5)), view.y + kScreenHeight};
                particle.shakeRange = random(10); // sway range 0..9 pixels
                break;
        }
    }

    void SceneEffect::move(SceneParticle& particle, std::uint64_t nowMs) {
        const int step = roundHalfEven(static_cast<double>(nowMs - particle.tickMs) * speed(particle));
        const double distX = std::abs(particle.target.x - particle.position.x);
        const double distY = std::abs(particle.target.y - particle.position.y);
        const auto nextFrame = [&] {
            if (nowMs - particle.tickMs >= static_cast<std::uint64_t>(particle.frameDelay)) {
                particle.frame = particle.frame + 1 > lastFrame(m_kind) ? 0 : particle.frame + 1;
                particle.tickMs = nowMs;
            }
        };
        if (m_kind == SceneEffectKind::Rain) {
            if (nowMs - particle.tickMs >= static_cast<std::uint64_t>(particle.frameDelay)) {
                if (particle.arrived) {
                    if (particle.frameDelay / 2 < static_cast<int>(nowMs % static_cast<std::uint64_t>(particle.frameDelay))) {
                        ++particle.frame;
                    }
                    if (particle.frame > 3) { // splash frames run 1..3
                        particle.frame = 1;
                    }
                } else {
                    particle.frame = 0;
                    particle.tickMs = nowMs;
                }
            }
        } else if (m_kind == SceneEffectKind::Stars) {
            nextFrame();
        }
        if (step > distX && step > distY) {
            particle.position = particle.target;
            particle.arrived = true;
            return;
        }
        double ratioYX = 0;
        double ratioXY = 0;
        if (distY > distX) {
            ratioYX = distY / (distX + 0.001); // 0.001: epsilon that avoids dividing by zero
        } else {
            ratioXY = distX / (distY + 0.001); // 0.001: epsilon that avoids dividing by zero
        }
        const double stepX = std::min<double>(step, step / (ratioYX + 0.001)); // 0.001: epsilon that avoids dividing by zero
        const double stepY = std::min<double>(step, step / (ratioXY + 0.001)); // 0.001: epsilon that avoids dividing by zero
        int dx = 0;
        int dy = 0;
        if (particle.position.x < particle.target.x) {
            dx = roundHalfEven(stepX);
        } else if (particle.position.x > particle.target.x) {
            dx = -roundHalfEven(stepX);
        }
        if (particle.position.y < particle.target.y) {
            dy = roundHalfEven(stepY);
        } else if (particle.position.y > particle.target.y) {
            dy = -roundHalfEven(stepY);
        }
        particle.position.x += dx;
        particle.position.y += dy;
        if (m_kind != SceneEffectKind::Rain && m_kind != SceneEffectKind::Stars) {
            nextFrame();
        }
    }

    void SceneEffect::sway(SceneParticle& particle, std::uint64_t nowMs) {
        const bool leaf = m_kind == SceneEffectKind::Leaves || m_kind == SceneEffectKind::FallingLeaves;
        const std::uint64_t sinceTick = nowMs - particle.tickMs;
        if (leaf && sinceTick < 50) { // 50: ms after a frame change before a leaf sways
            return;
        }
        const int quarter = particle.shakeRange / 4; // 4: a swing is made in quarter steps of the range
        if (particle.shakeRight) {
            switch (particle.shakePhase) {
                case 0: // phase 0: swinging out to the right
                    particle.shakeOffset += quarter;
                    if (particle.shakeOffset > particle.shakeRange) {
                        particle.shakePhase = 1;
                        particle.shakeOffset = particle.shakeRange;
                    }
                    break;
                case 1: // phase 1: holding at the right end (a leaf holds 200 ms)
                    if (!leaf || sinceTick >= 200) { // 200: ms a leaf holds at the end of a swing
                        particle.shakePhase = 2;
                    }
                    break;
                case 2: // phase 2: swinging back to the middle
                    particle.shakeOffset -= quarter;
                    if (particle.shakeOffset < 0) {
                        particle.shakeOffset = 0;
                        particle.shakeRight = false;
                        particle.shakePhase = 0;
                    }
                    break;
                default: break; // unknown phase: nothing
            }
            return;
        }
        switch (particle.shakePhase) {
            case 0: // phase 0: swinging out to the left
                particle.shakeOffset -= quarter;
                if (-particle.shakeRange > particle.shakeOffset) {
                    particle.shakePhase = 1;
                    particle.shakeOffset = -particle.shakeRange;
                }
                break;
            case 1: // phase 1: holding at the left end (a leaf holds 200 ms)
                if (!leaf || sinceTick >= 200) { // 200: ms a leaf holds at the end of a swing
                    particle.shakePhase = 2;
                }
                break;
            case 2: // phase 2: swinging back to the middle
                particle.shakeOffset += quarter;
                if (particle.shakeOffset >= 0) {
                    particle.shakeOffset = 0;
                    particle.shakeRight = true;
                    particle.shakePhase = 0;
                }
                break;
            default: break; // unknown phase: nothing
        }
    }

    void SceneEffect::update(core::Point view, std::uint64_t nowMs) {
        if (nowMs - m_spawnMs >= spawnIntervalMs(m_kind)) {
            int started = 0;
            for (SceneParticle& particle : m_particles) {
                if (particle.active) {
                    continue;
                }
                spawn(particle, view, nowMs);
                particle.active = true;
                m_spawnMs = nowMs;
                if (++started >= spawnsAtOnce(m_kind)) {
                    break;
                }
            }
        }
        for (SceneParticle& particle : m_particles) {
            if (!particle.active) {
                continue;
            }
            move(particle, nowMs);
            if (particle.arrived && nowMs - particle.tickMs >= lingerMs(m_kind)) {
                particle.active = false;
            }
        }
    }

    void SceneEffect::draw(gfx::Canvas& canvas, SceneParticle& particle, core::Point view, std::uint64_t nowMs) {
        if (swaying() && canvas.isAnimationFrame()) {
            sway(particle, nowMs);
        }
        const std::size_t index = static_cast<std::size_t>(particle.picture);
        const gfx::PictureId picture = m_pictures[index];
        if (!picture.isValid()) {
            return;
        }
        const core::Size size = m_frameSizes[index];
        const core::Rect frame{0, particle.frame * size.height, size.width, size.height};
        const core::Point at{particle.position.x + (particle.arrived ? 0 : particle.shakeOffset) - view.x, particle.position.y - view.y};
        if (m_kind == SceneEffectKind::Steam) {
            constexpr int kSteamLight = 10; // steam light blend strength (0..255), faint
            canvas.drawPictureLight(picture, at, kSteamLight, frame);
        } else if (m_kind == SceneEffectKind::Stars) {
            constexpr int kStarLight = 200; // star light blend strength (0..255), bright
            canvas.drawPictureLight(picture, at, kStarLight, frame);
        } else {
            canvas.drawPicture(picture, at, frame);
        }
    }

    void SceneEffect::drawUnder(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs) {
        const bool landed = m_kind != SceneEffectKind::Ribbons;
        const bool seen = m_kind == SceneEffectKind::Leaves || m_kind == SceneEffectKind::FallingLeaves || m_kind == SceneEffectKind::Rain ||
                          m_kind == SceneEffectKind::Stars || m_kind == SceneEffectKind::Ribbons;
        for (SceneParticle& particle : m_particles) {
            if (seen && particle.active && particle.arrived == landed) {
                draw(canvas, particle, view, nowMs);
            }
        }
    }

    void SceneEffect::drawOver(gfx::Canvas& canvas, core::Point view, std::uint64_t nowMs) {
        for (SceneParticle& particle : m_particles) {
            if (particle.active && !particle.arrived) {
                draw(canvas, particle, view, nowMs);
            }
        }
    }

}
