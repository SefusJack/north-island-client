#include "client/subsystems/minigames/SheepGame.h"

#include "core/Between.h"
#include "core/Rect.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kLives = 3; // hearts at the start
        constexpr int kTimeLeft = 30; // seconds of play
        constexpr int kGameOverDelay = 5; // seconds the game over state shows before finishing
        constexpr int kHammerStartTick = 10; // hammer tick at open: hammer swing ticks only run to 10, so no swing is in progress
        constexpr int kDreamRiseStart = 100; // start height of the rising dream bubble, px
        constexpr int kDreamRiseStep = 20; // px the dream bubble rises per frame
        constexpr int kDreamKinds = 6; // random dream kinds 0 to 5 (4 pictures per kind)
        constexpr int kFirstDreamTicks = 25; // ticks a dream lasts at the start
        constexpr int kEffectTicksPerFrame = 5; // ticks per hit effect frame
        constexpr int kEffectLastFrame = 4; // last frame of the hit effect (0 to 4)
        constexpr std::uint64_t kSecondMs = 1000; // one second, ms
        constexpr std::uint64_t kBlinkMs = 500; // sleeper blink period, ms
        constexpr int kSleeping = 26; // action slot 26: sleeper lying asleep
        constexpr int kBeaten = 28; // action slot 28: sleeper beaten (lost all hearts)
        // clickable areas of the sleeper picture as {x, y, width, height}, px; a click in any of them counts as a hit
        constexpr core::Rect kHitAreas[3] = {{307, 234, 110, 97}, {443, 199, 72, 57}, {482, 255, 25, 60}};
        const char* const kMusic = "BGM0019";

        /// @brief Tells whether a point lies in a rectangle.
        /// @param area The rectangle.
        /// @param at The point.
        /// @return True when inside.
        bool inArea(const core::Rect& area, core::Point at) { return area.contains(at); }

    }

    SheepGame::SheepGame(
        Hooks hooks,
        int gameKind
    )
        : m_hooks(std::move(hooks)),
          m_gameKind(gameKind) {}

    void SheepGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_started = false;
        m_finished = false;
        m_gameOver = false;
        m_lives = kLives;
        m_score = 0;
        m_timeLeft = kTimeLeft;
        m_dreamKind = 0;
        m_dreamTick = 0;
        m_dreamRise = 0;
        m_previousDreamRise = 0;
        m_dreamHit = false;
        m_hammerTick = kHammerStartTick;
        m_hammerFrame = 0;
        m_effectActive = false;
        m_hooks.stopMusic();
    }

    void SheepGame::begin(std::uint64_t nowMs) {
        m_started = true;
        m_hooks.playMusic(kMusic);
        m_sleeperSlot = kSleeping;
        m_sleeperAnim = HumanFrame{1, 0};
        m_secondMs = nowMs;
    }

    void SheepGame::leave() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void SheepGame::clicked(core::Point at) {
        if (m_gameOver || !m_started || m_hammerFrame != 0) {
            return;
        }
        if (!inArea(kHitAreas[0], at) && !inArea(kHitAreas[1], at) && !inArea(kHitAreas[2], at)) {
            return;
        }
        m_hooks.playSound("sound/SEB0027.wav"); // hammer hit sound
        m_hammerTick = 0;
        m_hammerFrame = 0;
        m_effectActive = true;
        m_effectFrame = 0;
        if (!m_dreamHit) {
            switch (m_dreamKind % 4) { // 4 dream pictures per random kind
                case 0: // dream kind 0: bad dream, hitting it costs a heart
                case 2: // dream kind 2: bad dream, hitting it costs a heart
                    --m_lives;
                    break;
                default: // dream kinds 1 and 3: good dreams, a point
                    ++m_score;
                    break;
            }
        }
        m_dreamHit = true;
    }

    void SheepGame::updateTarget() {
        if (m_gameOver) {
            return;
        }
        ++m_hammerTick;
        ++m_dreamTick;
        // hammer frame by ticks since the hit: up to 3 frame 0, 4-7 frame 1, 8-10 frame 2, above 10 back up (0)
        if (m_hammerTick > 10) {
            m_hammerFrame = 0;
        } else if (m_hammerTick > 7) {
            m_hammerFrame = 2;
        } else if (m_hammerTick > 3) {
            m_hammerFrame = 1;
        } else {
            m_hammerFrame = 0;
        }
        // a dream lasts 25 ticks, 1 fewer for every 6 seconds elapsed; good dreams are kinds 1 and 3 of 4 (a missed one costs a heart)
        if (kFirstDreamTicks - (kTimeLeft - m_timeLeft) / 6 <= m_dreamTick) {
            if (!m_dreamHit && (m_dreamKind % 4 == 1 || m_dreamKind % 4 == 3)) {
                --m_lives;
            }
            m_dreamKind = random(kDreamKinds);
            m_dreamTick = 0;
            m_dreamHit = false;
            m_dreamRise = kDreamRiseStart;
        }
    }

    void SheepGame::updateEffect() {
        if (m_gameOver) {
            return;
        }
        ++m_effectTick;
        if (m_effectTick == kEffectTicksPerFrame) {
            ++m_effectFrame;
            m_effectTick = 0;
        }
        if (m_effectFrame > kEffectLastFrame) {
            m_effectFrame = 0;
            m_effectActive = false;
        }
    }

    void SheepGame::drawStep(std::uint64_t nowMs) {
        if (!m_started) {
            return;
        }
        m_blinkFrame = nowMs - m_secondMs > kBlinkMs ? 1 : 0;
        if (!m_gameOver || m_lives > 0) {
            m_sleeperFrame = m_sleeperAnim.next(nowMs, m_hooks.sleeperFrames(m_sleeperSlot), true);
        } else {
            m_sleeperSlot = kBeaten;
            m_sleeperAnim.frame = 0;
            m_sleeperFrame = m_sleeperAnim.next(nowMs, m_hooks.sleeperFrames(m_sleeperSlot), true);
        }
        if (m_score < 0) {
            m_score = 0;
        }
    }

    void SheepGame::frame(std::uint64_t nowMs) {
        if (!m_started) {
            return;
        }
        if (m_dreamRise > 0) {
            m_dreamRise -= kDreamRiseStep;
        }
        if (nowMs - m_secondMs >= kSecondMs) {
            m_secondMs = nowMs;
            if (m_gameOver) {
                --m_gameOverDelay;
                if (m_gameOverDelay > 0) {
                    return;
                }
                m_finished = true;
                m_hooks.stopMusic();
                if (m_lives <= 0) {
                    m_hooks.endGame();
                } else {
                    m_hooks.finishGame(1); // result 1: finished with a heart left
                }
                return;
            }
            --m_timeLeft;
        }
        if (!m_gameOver && (m_timeLeft <= 0 || m_lives <= 0)) {
            m_gameOver = true;
            m_gameOverDelay = kGameOverDelay;
            return;
        }
        updateTarget();
        if (m_effectActive) {
            updateEffect();
        }
    }

    int SheepGame::dreamRise(double amount) const {
        if (std::abs(m_dreamRise - m_previousDreamRise) > kDreamRiseStep) {
            return m_dreamRise;
        }
        return core::between(m_previousDreamRise, m_dreamRise, amount);
    }

    void SheepGame::update(std::uint64_t nowMs) {
        constexpr int kMostCatchUp = 10; // max frames processed per call when behind
        for (int frames = 0; !m_finished && nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_previousDreamRise = m_dreamRise;
            frame(m_lastFrameMs);
            drawStep(m_lastFrameMs);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
