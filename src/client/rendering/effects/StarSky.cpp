#include "client/rendering/effects/StarSky.h"

#include "client/data/StarPositions.h"
#include "core/Between.h"
#include "core/ByteReader.h"
#include "gfx/PictureLibrary.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kSkyWidth = 832; // width of the Star_Sky picture in px; two copies scroll side by side
        constexpr int kLightStrength = 30; // light blend strength (0..255) of the star pictures and the flight light
        constexpr std::uint64_t kSkyScrollMs = 20; // the sky scrolls 1 px every 20 ms
        constexpr int kFadeSteps = 12; // black fade alpha per fade level (level * 12)
        constexpr int kMostFadeLevel = 19; // the fade-out stops growing past level 19 (alpha 228)
        constexpr core::Point kOwnStarPlace{300, 200}; // top-left of the player's own star on page 1, px
        constexpr core::Point kFlightTarget{416, 300}; // where the flying star ends: centre of the 832 x 600 sky, px
        constexpr std::uint64_t kRiseMs = 60; // the rising star moves every 60 ms
        constexpr int kRiseStep = 10; // ... by 10 px
        constexpr std::uint64_t kShineMs = 300; // one shine step lasts 300 ms
        constexpr int kShineFrames = 5; // after 5 shine steps (1.5 s) the screen fades
        constexpr double kFlySpeed = 0.2; // flying star speed, px per ms
        constexpr int kLayerStep = 2; // the star layer drifts right 2 px per animation frame
        constexpr int kSnapPixels = 160; // moves longer than 160 px per axis between beats are jumps, drawn without interpolation

        /// @brief The picture name of a star.
        /// @param star Star number.
        /// @return "Star_" followed by the number.
        std::string starName(int star) { return "Star_" + std::to_string(star); }

    }

    StarSky::StarSky(
        GameServices& services,
        Hooks hooks
    )
        : m_services(services),
          m_hooks(std::move(hooks)) {}

    std::optional<gfx::PictureId> StarSky::starPicture(int star) {
        if (star < 1 || star > 28) { // 28: stars that have a picture (Star_1..Star_28)
            return std::nullopt;
        }
        return m_services.pictures.find(starName(star));
    }

    void StarSky::loadOwned(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int count = reader.u8();
        for (int i = 0; i < count && !reader.failed(); ++i) {
            const int star = reader.u8();
            if (star > kStars) {
                m_ownStar = star - kStars;
            } else if (star >= 1) {
                m_owned[static_cast<std::size_t>(star)] = true;
            }
        }
    }

    void StarSky::acquired(int star, int npcIndex, std::uint64_t nowMs) {
        m_flyStar = star;
        if (!m_hooks.fighting()) {
            startFlight(star, npcIndex, nowMs);
        }
    }

    int StarSky::layoutSlot(int star) {
        switch (star) {
            case 5: return 1; // star 5 sits in layout slot 1
            case 9: return 2; // star 9 sits in layout slot 2
            case 1: return 3; // star 1 sits in layout slot 3
            case 4: return 4; // star 4 sits in layout slot 4
            case 3: return 5; // star 3 sits in layout slot 5
            case 8: return 6; // star 8 sits in layout slot 6
            case 7: return 7; // star 7 sits in layout slot 7
            case 6:
                return 8; // star 6 sits in layout slot 8
            // stars 12, 14, 16, 19, 20, 23, 24 and 27 use their own number as the layout slot
            case 12:
            case 14:
            case 16:
            case 19:
            case 20:
            case 23:
            case 24:
            case 27: return star;
            default: // other stars have no slot (0)
                return 0;
        }
    }

    core::Point StarSky::position(int star) const {
        constexpr int kPage = 1; // the book page shown (only page 1 is used)
        constexpr int kPageWidth = 400; // horizontal shift between book pages, px
        if (star == kOwnStar) {
            return {kOwnStarPlace.x + (kPage - 1) * kPageWidth, kOwnStarPlace.y};
        }
        const int slot = layoutSlot(star);
        core::Point at = StarPositions::layout(slot);
        if (slot >= 1 && slot <= 8) { // page 1 holds slots 1..8
            at.x += (kPage - 1) * kPageWidth;
        } else if (slot >= 9 && slot <= 14) { // page 2 holds slots 9..14
            at.x += (kPage - 2) * kPageWidth;
        } else if (slot >= 15 && slot <= 20) { // page 3 holds slots 15..20
            at.x += (kPage - 3) * kPageWidth;
        } else if (slot >= 21 && slot <= 26) { // page 4 holds slots 21..26
            at.x += (kPage - 4) * kPageWidth;
        } else if (slot >= 27 && slot <= 32) { // page 5 holds slots 27..32
            at.x += (kPage - 5) * kPageWidth;
        }
        return at;
    }

    void StarSky::open(std::uint64_t nowMs) {
        m_offsets.fill(0);
        m_ownStarOffset = 0;
        m_hoverStar = 0;
        m_hooks.startMusic();
        m_hooks.quitButton(true);
        m_skyMs = nowMs;
        m_phase = Phase::Sky;
    }

    void StarSky::quit() {
        m_fadeState = 1;
        m_fadeLevel = 0;
        m_hooks.quitButton(false);
    }

    void StarSky::startFlight(int star, int npcIndex, std::uint64_t nowMs) {
        m_flyStar = star;
        m_flyOwn = npcIndex == 0;
        const auto picture = starPicture(star);
        if (!picture) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*picture);
        const int width = data.width();
        const int height = data.height() / 2; // the picture stacks two frames: plain and lit
        if (m_flyOwn) {
            m_ownStar = 0;
        }
        const auto from = npcIndex == 0 ? m_hooks.ownPoint() : m_hooks.npcPoint(npcIndex);
        if (!from) {
            return;
        }
        m_flyFrom = {from->x - width / 2, from->y - height / 2};
        m_fly = m_flyFrom;
        m_flyTo = {kFlightTarget.x - width / 2, kFlightTarget.y - height / 2};
        m_flySource = {0, 0, width, height};
        m_phase = Phase::Rising;
        if (const auto light = m_services.pictures.find("S10231")) {
            LightEffect::Setup setup;
            setup.picture = *light;
            setup.frameMs = 70; // light frame time in ms
            setup.firstFrame = 1;
            setup.lastFrame = 16; // S10231 has 16 frames
            setup.frameCount = 16; // S10231 has 16 frames
            setup.motion = LightEffect::Motion::Once;
            setup.loop = false;
            setup.alpha = kLightStrength;
            m_light.start(setup, {m_flyFrom.x + 100, m_flyFrom.y + 250}, nowMs); // the light starts 100 pixels right of and 250 below the star's top-left
        }
        m_hooks.hud(false);
        m_services.playSound("Sound/WAV0149.wav"); // star flight sound
        m_hooks.startMusic();
        m_flightMs = nowMs;
    }

    void StarSky::moveFlyingStar(std::uint64_t nowMs) {
        int step = static_cast<int>(std::lround(kFlySpeed * static_cast<double>(nowMs - m_moveXMs)));
        m_moveXMs = nowMs;
        int x = m_fly.x;
        if (m_flyDistX >= m_flyDistY) {
            if (m_flyFrom.x < m_flyTo.x) {
                x = std::min(x + step, m_flyTo.x);
            } else if (m_flyFrom.x > m_flyTo.x) {
                x = std::max(x - step, m_flyTo.x);
            }
        } else if (m_flyFrom.x != m_flyTo.x) {
            const int share = static_cast<int>(std::lround(m_flyDistX / (m_flyDistY + 0.0001) * step)); // 0.0001: epsilon that avoids dividing by zero
            x = m_flyFrom.x < m_flyTo.x ? std::min(x + share, m_flyTo.x) : std::max(x - share, m_flyTo.x);
        }
        step = static_cast<int>(std::lround(kFlySpeed * static_cast<double>(nowMs - m_moveYMs)));
        m_moveYMs = nowMs;
        int y = m_fly.y;
        if (m_flyDistX >= m_flyDistY) {
            if (m_flyFrom.y != m_flyTo.y) {
                const int share = static_cast<int>(std::lround(m_flyDistY / (m_flyDistX + 0.0001) * step)); // 0.0001: epsilon that avoids dividing by zero
                y = m_flyFrom.y < m_flyTo.y ? std::min(y + share, m_flyTo.y) : std::max(y - share, m_flyTo.y);
            }
        } else if (m_flyFrom.y < m_flyTo.y) {
            y = std::min(y + step, m_flyTo.y);
        } else if (m_flyFrom.y > m_flyTo.y) {
            y = std::max(y - step, m_flyTo.y);
        }
        m_fly = {x, y};
    }

    void StarSky::updateFlight(std::uint64_t nowMs) {
        const auto picture = starPicture(m_flyStar);
        if (!picture) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*picture);
        const int height = data.height() / 2; // the picture stacks two frames: plain and lit
        const auto landed = [&] {
            m_flyFrame = 0;
            m_phase = Phase::Shining;
            m_fly = m_flyTo;
            m_flySource = {0, height, data.width(), height};
            if (m_flyOwn) {
                m_ownStar = m_flyStar;
            } else {
                m_owned[static_cast<std::size_t>(m_flyStar)] = true;
            }
            m_flightMs = nowMs;
        };
        switch (m_phase) {
            case Phase::Rising: // phase 1: the star rises off the top of the screen
                if (nowMs - m_flightMs >= kRiseMs) {
                    m_fly.y -= kRiseStep;
                    m_flightMs = nowMs;
                    if (height + m_fly.y <= 0) {
                        m_phase = Phase::Flying;
                        m_flyFrom.y = 600; // 600: the star re-enters at the bottom of the 800x600 screen
                        m_fly = m_flyFrom;
                        m_skyX = -16; // start offset of the sky layer in pixels
                        m_skyY = -20; // start offset of the sky layer in pixels
                        m_flyDistX = std::abs(m_flyTo.x - m_flyFrom.x);
                        m_flyDistY = std::abs(m_flyTo.y - m_flyFrom.y);
                        m_moveXMs = nowMs;
                        m_moveYMs = nowMs;
                        m_skyMs = nowMs;
                    }
                }
                break;
            case Phase::Flying: // phase 2: the star flies from the bottom of the sky to its middle
                moveFlyingStar(nowMs);
                if (m_flyDistX >= m_flyDistY ? m_fly.x == m_flyTo.x : m_fly.y == m_flyTo.y) {
                    landed();
                }
                break;
            case Phase::Shining: // phase 3: the star shines for a moment, then the screen fades
                if (nowMs - m_flightMs >= kShineMs) {
                    m_flySource = {0, height, data.width(), height};
                    m_flightMs = nowMs;
                    if (++m_flyFrame == kShineFrames) {
                        m_fadeState = 1;
                        m_fadeLevel = 0;
                    }
                }
                break;
            default: // None and Sky: no flight to update
                break;
        }
    }

    void StarSky::drawFlyingStar(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (canvas.isAnimationFrame()) {
            m_flyBefore = m_fly;
            m_flyStepMs = canvas.beatMs();
            updateFlight(nowMs);
        }
        m_light.draw(canvas, nowMs);
        core::Point at = m_fly;
        if (const auto amount = canvas.betweenBeats(); amount && m_flyStepMs >= canvas.beatMs()) {
            at = core::between(m_flyBefore, m_fly, *amount, kSnapPixels);
        }
        if (const auto picture = starPicture(m_flyStar)) {
            canvas.drawPictureLight(*picture, at, kLightStrength, m_flySource);
        }
    }

    void StarSky::drawSkyBackground(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (canvas.isAnimationFrame() && nowMs - m_skyMs >= kSkyScrollMs) {
            m_skyXBefore = m_skyX;
            m_skyStepMs = canvas.beatMs();
            if (++m_skyX >= kSkyWidth) {
                m_skyX = 0;
            }
            m_skyMs = nowMs;
        }
        int x = m_skyX;
        if (const auto amount = canvas.betweenBeats(); amount && m_skyStepMs >= canvas.beatMs()) {
            x = core::between(core::Point{m_skyXBefore, m_skyY}, core::Point{m_skyX, m_skyY}, *amount, kSnapPixels).x;
        }
        if (const auto sky = m_services.pictures.find("Star_Sky")) {
            canvas.drawPicture(*sky, {x, m_skyY});
            canvas.drawPicture(*sky, {x - kSkyWidth, m_skyY});
        }
    }

    void StarSky::hitTest(core::Point mouse) {
        m_hoverStar = 0;
        for (int star = 1; star <= kOwnStar; ++star) {
            const bool own = star == kOwnStar;
            if (own ? m_ownStar == 0 : !m_owned[static_cast<std::size_t>(star)]) {
                continue;
            }
            const auto picture = starPicture(own ? m_ownStar : star);
            if (!picture) {
                continue;
            }
            const gfx::Picture& data = m_services.pictures.picture(*picture);
            const core::Point at = position(star);
            const int offset = own ? m_ownStarOffset : m_offsets[static_cast<std::size_t>(star)];
            if (mouse.x >= at.x + offset && mouse.x <= at.x + offset + data.width() && mouse.y >= at.y &&
                mouse.y <= at.y + data.height() / 2) { // the picture stacks two frames: plain and lit
                m_hoverStar = star;
                return;
            }
        }
    }

    void StarSky::drawStars(gfx::Canvas& canvas) {
        constexpr int kWrapAt = 1000; // a star drifted to x >= 1000 px wraps around
        constexpr int kEntry = 1000; // a wrapped star is moved back by 1000 px, to the left of its slot
        int lag = 0;
        if (const auto amount = canvas.betweenBeats(); amount && m_layerStepMs >= canvas.beatMs()) {
            lag = core::between(-kLayerStep, 0, *amount);
        }
        for (int star = 1; star <= kOwnStar; ++star) {
            const bool own = star == kOwnStar;
            if (own ? m_ownStar == 0 : !m_owned[static_cast<std::size_t>(star)]) {
                continue;
            }
            const auto picture = starPicture(own ? m_ownStar : star);
            if (!picture) {
                continue;
            }
            const core::Point at = position(star);
            int& offset = own ? m_ownStarOffset : m_offsets[static_cast<std::size_t>(star)];
            const gfx::Picture& data = m_services.pictures.picture(*picture);
            const int height = data.height() / 2; // the picture stacks two frames: plain and lit
            const core::Rect source{0, m_hoverStar == star ? height : 0, data.width(), height};
            if (!own && at.x == -1 && at.y == -1) { // (-1, -1): the layout has no place for this star
                continue;
            }
            const int drawnX = offset + lag + at.x;
            if (drawnX + data.width() > 0 && drawnX < 800) { // 800: screen width, only stars on screen are drawn
                canvas.drawPictureLight(*picture, {drawnX, at.y}, kLightStrength, source);
            }
            if (offset + at.x >= kWrapAt) {
                offset = -kEntry - at.x + 200; // 200: pixels in from the left edge where a wrapped star re-enters
            }
        }
    }

    void StarSky::updateFade(gfx::Canvas& canvas) {
        if (m_fadeState == 0) {
            return;
        }
        const auto fill = [&](int level) { canvas.fillRect(canvas.window(), core::Color{0, 0, 0, static_cast<std::uint8_t>(level * kFadeSteps)}); };
        if (!canvas.isAnimationFrame()) {
            const auto amount = canvas.betweenBeats();
            fill(amount && m_fadeStepMs >= canvas.beatMs() ? core::between(m_fadeFrom, m_fadeLevel, *amount) : m_fadeLevel);
            return;
        }
        fill(m_fadeLevel);
        m_fadeFrom = m_fadeLevel;
        m_fadeStepMs = canvas.beatMs();
        if (m_fadeState == 1) {
            if (++m_fadeLevel > kMostFadeLevel) {
                m_fadeState = 2;
            }
            return;
        }
        if (--m_fadeLevel < 1) {
            m_fadeState = 0;
            m_fadeLevel = 0;
            m_flyStar = 0;
            m_phase = Phase::None;
            m_hooks.quitButton(false);
            m_hooks.restoreMusic();
        }
    }

    void StarSky::paint(gfx::Canvas& canvas, std::uint64_t nowMs, core::Point mouse) {
        switch (m_phase) {
            case Phase::Rising: // phase 1: only the flying star
                drawFlyingStar(canvas, nowMs);
                break;
            case Phase::Flying: // phase 2: sky and flying star, then the fade
            case Phase::Shining: // phase 3: sky and shining star, then the fade
                if (m_fadeState != 2) { // fade state 2: the sky is gone, the map shows again
                    drawSkyBackground(canvas, nowMs);
                    drawFlyingStar(canvas, nowMs);
                } else {
                    m_hooks.hud(true);
                }
                updateFade(canvas);
                break;
            case Phase::Sky: // phase 4: the sky opened for looking
                if (m_fadeState != 2) {
                    if (canvas.isAnimationFrame()) {
                        for (std::size_t i = 1; i < m_offsets.size(); ++i) {
                            m_offsets[i] += kLayerStep;
                        }
                        m_ownStarOffset += kLayerStep;
                        m_layerStepMs = canvas.beatMs();
                    }
                    hitTest(mouse);
                    drawSkyBackground(canvas, nowMs);
                    drawStars(canvas);
                } else {
                    m_hooks.hud(true);
                }
                updateFade(canvas);
                break;
            case Phase::None: // no star book on screen
                break;
        }
    }

    void StarSky::rightReleased() {
        if (m_phase == Phase::Sky && !m_roleInfoShown && m_fadeState == 0) { // fade state 0: no fade running
            m_fadeState = 1;
            m_fadeLevel = 0;
        }
    }

    void StarSky::battleStarted() {
        if (m_phase == Phase::None) {
            return;
        }
        m_phase = Phase::None;
        if (m_flyOwn) {
            m_ownStar = m_flyStar;
        } else if (m_flyStar >= 1 && m_flyStar <= kStars) {
            m_owned[static_cast<std::size_t>(m_flyStar)] = true;
        }
    }

}
