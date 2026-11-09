#include "client/subsystems/minigames/PumpkinGame.h"

#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kFieldWidth = 832; // width of the play field, px
        constexpr int kBottom = 600; // y of the ground line the ghosts walk on, px
        constexpr int kTurnTop = 230; // y at which a rising ghost turns round and descends, px
        constexpr int kGhostSpacing = kFieldWidth / 15; // horizontal spacing between ghost columns, px
        constexpr int kGhostFeetRight = 15; // x offset from ghost position to where its picture is drawn, px
        constexpr int kGhostFeetDown = 76; // y offset from ghost position to where its picture is drawn, px
        constexpr int kFallSpeed = 8; // px a hit ghost falls per update
        constexpr int kWalking = 4; // ghost action slot 4: walking down (with coin)
        constexpr int kRising = 0; // ghost action slot 0: rising out of the ground
        constexpr int kFallen = 26; // ghost action slot 26: falling/lying on its back after a hit
        constexpr int kTargetWidth = 30; // ghost hit box width, px
        constexpr int kTargetHeight = 60; // ghost hit box height, px
        constexpr int kHitMargin = 20; // extra hit margin around the ghost, px
        constexpr int kShotSpeed = 15; // px the pumpkin shot moves per update
        constexpr int kShotLightDrop = 30; // px below the shot where the hit light appears
        constexpr int kAimRadius = 50; // distance of the aim point from the player, px
        constexpr int kBarbRadius = 40; // distance of the aim arrow barbs from the player, px
        constexpr double kBarbAngle = 0.5; // angle of the aim arrow barbs either side of the aim, radians
        constexpr double kAimStep = 0.06; // aim sweep per update, radians
        constexpr double kAimLimit = 1.6; // aim sweep limit either side of straight down, radians
        constexpr int kSpawnEvery = 20; // a ghost starts rising every this many ticks
        constexpr int kTickPeriod = 40; // tick counter wraps after this many ticks
        constexpr int kSpawnChoices = 10; // ghost index is drawn random(10), i.e. 0 to 9
        constexpr int kWinScore = 30; // ghosts to hit to win
        constexpr int kStartHearts = 3; // lives at the start
        constexpr int kStartTime = 50; // seconds of play
        constexpr int kStartCountdown = 4; // countdown start value (shows 3, 2, 1, Go)
        constexpr std::uint64_t kSecondMs = 1000; // one second, ms
        constexpr std::uint64_t kOverMs = 3000; // how long the game over text shows, ms
        const char* const kShotPicture = "1268"; // picture 1268: the pumpkin shot
        const char* const kMusic = "BGM0019"; // game music

        /// @brief Tells whether an action slot is one of the lying (fallen) ghost groups.
        /// @param slot Action slot.
        /// @return True for slots 26 and 27.
        bool isLyingGroup(int slot) { return slot == 26 || slot == 27; } // action slots 26/27: ghost lying on its back

        /// @brief Rounds a double to the nearest int.
        /// @param value Value to round.
        /// @return The rounded value.
        int rounded(double value) { return static_cast<int>(std::nearbyint(value)); }

    }

    PumpkinGame::PumpkinGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void PumpkinGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_countdownTickMs = nowMs;
        m_formShown = false;
        m_phase = Phase::Countdown;
        m_score = 0;
        m_hearts = kStartHearts;
        m_timeLeft = kStartTime;
        m_startCountdown = kStartCountdown;
        m_tick = 0;
        m_reverse = false;
        m_bomb = Bomb{};
        m_bombShown = false;
        m_playerAnim = HumanFrame{};
        m_playerFrame = 0;
        for (int i = 0; i <= kGhosts; ++i) {
            PumpkinGhost& ghost = m_ghosts[static_cast<std::size_t>(i)];
            ghost = PumpkinGhost{};
            // ghosts 0..5 are spaced from the left edge, the rest from the right edge (5 = last left-side ghost)
            ghost.pos = {i <= 5 ? kGhostSpacing * i : kFieldWidth - kGhostSpacing * (i - 5 + 1), kBottom};
            ghost.slot = kWalking;
        }
    }

    void PumpkinGame::exitGame() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void PumpkinGame::finish(int result) {
        m_hooks.stopMusic();
        m_hooks.finishGame(result);
    }

    std::string PumpkinGame::overText() const { return m_hearts != 0 && m_score >= kWinScore ? "You Win" : "You Lose"; }

    void PumpkinGame::countdownStep(std::uint64_t nowMs) {
        if (m_phase != Phase::Countdown) {
            return;
        }
        if (nowMs - m_countdownTickMs >= kSecondMs || m_startCountdown < 0) {
            if (m_startCountdown < 0) {
                m_phase = Phase::Playing;
                m_hooks.playMusic(kMusic);
            }
            --m_startCountdown;
            if (m_startCountdown > 0) {
                m_hooks.playSound("sound/wav0145.wav"); // countdown tick sound
            }
            if (m_startCountdown == 0) {
                m_hooks.playSound("sound/wav1610.wav"); // "Go" sound
            }
            m_countdownTickMs = nowMs;
        }
    }

    void PumpkinGame::tickClock(std::uint64_t nowMs) {
        if (m_phase == Phase::Playing && nowMs - m_countdownTickMs >= kSecondMs) {
            --m_timeLeft;
            m_countdownTickMs = nowMs;
            if (m_timeLeft < 0) {
                m_phase = Phase::Countdown;
            }
        }
    }

    void PumpkinGame::mouseDown() {
        if (m_bomb.flying) {
            return;
        }
        m_slope = 0;
        m_slopeSum = 0;
        m_shotFrom = m_aim;
        m_shotDx = std::abs(kPlayerAt.x - m_aim.x);
        m_shotDy = std::abs(kPlayerAt.y - m_aim.y);
        if (m_shotDx > m_shotDy) {
            m_slope = static_cast<double>(m_shotDy) / m_shotDx;
        } else if (m_shotDy > m_shotDx) {
            m_slope = static_cast<double>(m_shotDx) / m_shotDy;
        } else {
            m_slope = 1;
        }
        m_bomb.flying = true;
    }

    void PumpkinGame::updateAim() {
        const auto at = [](double angle, int radius) {
            return core::Point{rounded(std::sin(angle) * radius + kPlayerAt.x), rounded(std::cos(angle) * radius + kPlayerAt.y)};
        };
        m_aim = at(m_angle, kAimRadius);
        m_arrowA = at(m_angle + kBarbAngle, kBarbRadius);
        m_arrowB = at(m_angle - kBarbAngle, kBarbRadius);
        if (!m_reverse) {
            m_angle += kAimStep;
            if (m_angle > kAimLimit) {
                m_reverse = true;
            }
        } else {
            m_angle -= kAimStep;
            if (m_angle < -kAimLimit) {
                m_reverse = false;
            }
        }
    }

    void PumpkinGame::updateShot() {
        // the pumpkin is spent outside the field: x below 10 or above 800 (left/right edges)
        if (m_bomb.pos.x < 10 || m_bomb.pos.x > 800 || m_bomb.pos.y > kBottom || m_bomb.hit) {
            m_bomb.flying = false;
            m_bomb.pos = m_aim;
            m_bomb.hit = false;
        }
        m_slopeSum += m_slope;
        if (!m_bomb.flying) {
            return;
        }
        if (m_shotDx > m_shotDy) {
            if (m_shotFrom.y > kPlayerAt.y) {
                m_bomb.pos.y = m_shotFrom.y + rounded(m_slopeSum * kShotSpeed);
            }
            m_bomb.pos.x += m_shotFrom.x > kPlayerAt.x ? kShotSpeed : -kShotSpeed;
        } else if (m_shotDy > m_shotDx) {
            if (m_shotFrom.y > kPlayerAt.y) {
                m_bomb.pos.y += kShotSpeed;
            }
            const int sideways = rounded(m_slopeSum * kShotSpeed);
            m_bomb.pos.x = m_shotFrom.x > kPlayerAt.x ? m_shotFrom.x + sideways : m_shotFrom.x - sideways;
        } else {
            if (m_shotFrom.y > kPlayerAt.y) {
                m_bomb.pos.y += kShotSpeed;
            }
            m_bomb.pos.x += m_shotFrom.x > kPlayerAt.x ? kShotSpeed : -kShotSpeed;
        }
        m_bombShown = true;
        m_bombShownAt = m_bomb.pos;
    }

    bool PumpkinGame::shotHits(const PumpkinGhost& ghost) const {
        // field edges: x 10 (left) and 800 (right)
        if (m_bomb.hit || m_bomb.pos.x <= 10 || m_bomb.pos.x >= 800 || m_bomb.pos.y >= kBottom) {
            return false;
        }
        const core::Size shot = m_hooks.pictureSize(kShotPicture);
        // the 10s inset the pumpkin picture's box on each side
        return ghost.pos.x + kTargetWidth + kHitMargin >= m_bomb.pos.x + 10 && kHitMargin + ghost.pos.x <= m_bomb.pos.x + shot.width - 10 &&
               ghost.pos.y + kTargetHeight + kHitMargin >= m_bomb.pos.y + 10 && kHitMargin + ghost.pos.y <= m_bomb.pos.y + shot.height - 10;
    }

    void PumpkinGame::updateGhost(int index, std::uint64_t nowMs) {
        PumpkinGhost& ghost = m_ghosts[static_cast<std::size_t>(index)];
        ghost.shown = PumpkinGhost::Shown::Nothing;
        const auto show = [&](int slot, bool coin) {
            ghost.slot = slot;
            ghost.shown = PumpkinGhost::Shown::Walking;
            ghost.shownAt = {ghost.pos.x + kGhostFeetRight, ghost.pos.y + kGhostFeetDown};
            ghost.shownSlot = slot;
            ghost.shownFrame = ghost.anim.next(nowMs, m_hooks.ghostFrames(slot), isLyingGroup(slot));
            ghost.shownCoin = coin;
        };
        if (ghost.active) {
            if (!ghost.hit) {
                if (ghost.descending) {
                    ++ghost.pos.y;
                    show(kWalking, true);
                } else {
                    --ghost.pos.y;
                    show(kRising, false);
                }
            } else if (ghost.descending && ghost.pos.y == kBottom) {
                ghost.shown = PumpkinGhost::Shown::Grave;
            } else {
                ghost.pos.y += kFallSpeed;
                show(kFallen, false);
            }
            if (ghost.pos.y > kBottom) {
                if (ghost.descending) {
                    if (m_hearts > 0 && !ghost.hit) {
                        --m_hearts;
                    }
                    ghost.pos.y = kBottom;
                    ghost.hit = true;
                } else {
                    ghost.active = false;
                    ghost.pos.y = kBottom;
                    ghost.hit = false;
                }
            }
            if (ghost.pos.y < kTurnTop) {
                ghost.descending = true;
            }
        }
        if (!shotHits(ghost)) {
            return;
        }
        if (!ghost.hit) {
            ++m_score;
            m_hooks.hitLight({m_bomb.pos.x, m_bomb.pos.y + kShotLightDrop});
            m_hooks.playSound("sound/Wav1611.wav"); // ghost hit sound
            m_bomb.hit = true;
            ghost.descending = false;
        }
        ghost.hit = true;
    }

    void PumpkinGame::updatePlay(std::uint64_t nowMs) {
        m_bombShown = false;
        for (PumpkinGhost& ghost : m_ghosts) {
            ghost.shown = PumpkinGhost::Shown::Nothing;
        }
        if (m_phase != Phase::Playing) {
            return;
        }
        if (!m_bomb.flying) {
            updateAim();
            m_bomb.pos = m_aim;
        }
        if (m_bomb.flying) {
            updateShot();
        }
        if (m_tick > kTickPeriod) {
            m_tick = 0;
        }
        ++m_tick;
        if (m_tick % kSpawnEvery == 0) {
            PumpkinGhost& chosen = m_ghosts[static_cast<std::size_t>(m_hooks.random(kSpawnChoices))];
            if (!chosen.active) {
                chosen.active = true;
            }
        }
        for (int index = 1; index <= kGhosts; ++index) {
            updateGhost(index, nowMs);
        }
    }

    void PumpkinGame::updateOnce(std::uint64_t nowMs) {
        if (!m_hooks.instructionsOpen()) {
            m_formShown = true;
            countdownStep(nowMs);
        }
        if (m_phase == Phase::Playing && (m_hearts == 0 || m_timeLeft == 0)) {
            m_phase = Phase::Over;
            m_endTickMs = nowMs;
        }
        updatePlay(nowMs);
        tickClock(nowMs);
    }

    void PumpkinGame::update(std::uint64_t nowMs) {
        constexpr int kMostCatchUp = 10; // max frames processed per call when behind
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_previousBombShown = m_bombShown;
            m_previousBombAt = m_bombShownAt;
            m_previousAim = m_aim;
            m_previousArrowA = m_arrowA;
            m_previousArrowB = m_arrowB;
            for (PumpkinGhost& ghost : m_ghosts) {
                ghost.previousShown = ghost.shown;
                ghost.previousShownAt = ghost.shownAt;
            }
            updateOnce(m_lastFrameMs);
            m_playerFrame = m_playerAnim.next(m_lastFrameMs, m_hooks.playerFrames(12)); // action slot 12 of the player's figure
            updateOnce(m_lastFrameMs);
            if (m_phase == Phase::Over && m_lastFrameMs - m_endTickMs >= kOverMs) {
                finish(m_hearts != 0 && m_score >= kWinScore ? 1 : 0); // result 1: won, 0: lost
                return;
            }
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
