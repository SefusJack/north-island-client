#include "client/subsystems/minigames/HitMouseGame.h"

namespace client {

    namespace {

        constexpr int kMouseNpc = 19007; // NPC 19007: Picky Mouse (mouse look)
        constexpr int kMouseNpcToo = 11000; // NPC 11000: Persian Cat (also uses the mouse look)
        constexpr int kTurtleNpc = 17928; // NPC 17928: Old Sea Turtle (turtle look)
        constexpr int kCountdownStart = 4; // start countdown, seconds before play begins
        constexpr std::uint64_t kSecondMs = 1000; // one second, ms
        constexpr std::uint64_t kEndWaitMs = 3000; // wait after time is out before the result, ms
        constexpr std::uint64_t kHammerMs = 50; // time per hammer swing phase, ms
        constexpr std::uint64_t kHitFrameMs = 200; // time per frame of the hit animation, ms
        constexpr int kBombChance = 75; // a new target is a bomb when random(101) is above this, percent (about 25 percent)
        constexpr int kBombPenalty = 3; // score lost when hitting a bomb, points
        const char* const kMusic = "BGM0019"; // background music name

        // base screen position {x, y} of each of the 7 holes (index 0 unused), px
        constexpr std::array<core::Point, 8> kBases{{{0, 0}, {300, 150}, {500, 150}, {200, 300}, {400, 300}, {600, 300}, {300, 450}, {500, 450}}};
        constexpr int kHoleShift = 50; // a hole's x is its base x minus this, px

    }

    HitMouseGame::HitMouseGame(
        GameHooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void HitMouseGame::open(int npcId, std::uint64_t nowMs) {
        m_look = npcId == kMouseNpc || npcId == kMouseNpcToo ? Look::Mouse : npcId == kTurtleNpc ? Look::Turtle : Look::Rabbit;
        m_holes = {};
        for (int index = 1; index <= kHoles; ++index) {
            Hole& hole = m_holes[static_cast<std::size_t>(index)];
            hole.position = {kBases[static_cast<std::size_t>(index)].x - kHoleShift, kBases[static_cast<std::size_t>(index)].y};
            hole.tickMs = nowMs;
        }
        m_timeLeft = kStartSeconds;
        m_countdown = kCountdownStart;
        m_state = 0; // game state 0: countdown before play
        m_score = 0;
        m_begun = false;
        m_hammer = Hammer::Ready;
        m_timerMs = nowMs;
        m_hooks.stopMusic();
    }

    void HitMouseGame::setSpeeds() {
        // stage thresholds in seconds left: 31, 21, 11 (every ten seconds); hold times and frame intervals per stage in ms
        const int stage = m_timeLeft >= 31 ? 0 : m_timeLeft >= 21 ? 1 : m_timeLeft >= 11 ? 2 : 3;
        constexpr int kHold[] = {600, 500, 400, 300}; // ms a target stays up, per stage
        constexpr int kInterval[] = {120, 100, 80, 60}; // ms per rise or fall animation frame, per stage
        m_holdMs.fill(kHold[stage]);
        m_intervalMs.fill(kInterval[stage]);
    }

    void HitMouseGame::spawn() {
        if (m_state != 1) { // game state 1: playing
            return;
        }
        int busy = 0;
        for (int index = 1; index <= kHoles; ++index) {
            if (m_holes[static_cast<std::size_t>(index)].state != 0) { // hole state 0: empty
                ++busy;
            }
        }
        // time phases at 40 and 20 seconds left; busy-hole limit 2/3/4 and at most 1/2/3 new targets per phase
        const bool first = m_timeLeft >= 40;
        const bool second = m_timeLeft >= 20 && m_timeLeft < 40;
        const int busyLimit = first ? 2 : second ? 3 : 4;
        const int most = first ? 1 : second ? 2 : 3;
        if (busy >= busyLimit) {
            return;
        }
        int started = 0;
        for (int tries = 1; tries <= kHoles; ++tries) {
            Hole& hole = m_holes[static_cast<std::size_t>(random(kHoles) + 1)];
            if (hole.state == 0) {
                hole.bomb = random(101) > kBombChance; // original Random(101): values 0..100
                hole.state = 1; // hole state 1: rising
                ++started;
            }
            if (started >= most) {
                return;
            }
        }
    }

    void HitMouseGame::animate(std::uint64_t nowMs) {
        for (int index = 1; index <= kHoles; ++index) {
            Hole& hole = m_holes[static_cast<std::size_t>(index)];
            const std::size_t at = static_cast<std::size_t>(index);
            switch (hole.state) {
                case 1: // hole state 1: rising
                    if (elapsed(hole.tickMs, static_cast<std::uint64_t>(m_intervalMs[at]), nowMs)) {
                        ++hole.animFrame;
                        if (hole.animFrame >= kTargetFrames) {
                            hole.animFrame = kTargetFrames;
                            hole.state = 3; // hole state 3: up
                        }
                        hole.tickMs = nowMs;
                    }
                    break;
                case 2: // hole state 2: going down
                    if (elapsed(hole.tickMs, static_cast<std::uint64_t>(m_intervalMs[at]), nowMs)) {
                        --hole.animFrame;
                        if (hole.animFrame <= 0) {
                            hole.animFrame = 0;
                            hole.state = 0;
                        }
                        hole.tickMs = nowMs;
                    }
                    break;
                case 4: // hole state 4: hit (target hit animation; a bomb empties at once)
                    if (hole.bomb) {
                        hole.hitFrame = 0;
                        hole.state = 0;
                        hole.tickMs = nowMs;
                    } else if (elapsed(hole.tickMs, kHitFrameMs, nowMs)) {
                        ++hole.hitFrame;
                        if (hole.hitFrame >= kTargetFrames) {
                            hole.hitFrame = 0;
                            hole.state = 0;
                        }
                        hole.tickMs = nowMs;
                    }
                    break;
                case 3: // hole state 3: up, held for the hold time
                    if (elapsed(hole.tickMs, static_cast<std::uint64_t>(m_holdMs[at]), nowMs)) {
                        hole.state = 2; // hole state 2: going down
                    }
                    break;
                default: break; // hole state 0: empty, nothing to animate
            }
        }
    }

    void HitMouseGame::tickTimer(std::uint64_t nowMs) {
        if (m_state == 1 && elapsed(m_timerMs, kSecondMs, nowMs)) { // game state 1: playing
            --m_timeLeft;
            m_timerMs = nowMs;
            if (m_timeLeft == 0) {
                m_state = 2; // game state 2: the time is out
            }
        }
    }

    void HitMouseGame::updateHammer(std::uint64_t nowMs) {
        if (m_hammer == Hammer::Down && elapsed(m_hammerMs, kHammerMs, nowMs)) {
            m_hammerMs = nowMs;
            m_hammer = Hammer::Up;
        } else if (m_hammer == Hammer::Up && elapsed(m_hammerMs, kHammerMs, nowMs)) {
            m_hammer = Hammer::Ready;
        }
    }

    void HitMouseGame::runCountdown(std::uint64_t nowMs) {
        if (m_state != 0 || !(elapsed(m_timerMs, kSecondMs, nowMs) || m_countdown < 0)) {
            return;
        }
        if (m_countdown < 0) {
            m_state = 1; // game state 1: playing
            m_hooks.playMusic(kMusic);
            return;
        }
        --m_countdown;
        m_timerMs = nowMs;
        if (m_countdown == 0) { // countdown reached 0: start sound
            m_hooks.playSound("sound/wav1610.wav");
        } else if (m_countdown > 0) {
            m_hooks.playSound("sound/wav1605.wav");
        }
    }

    void HitMouseGame::checkEnd(std::uint64_t nowMs) {
        if (m_state == 2 && elapsed(m_timerMs, kEndWaitMs, nowMs)) { // game state 2: the time is out
            finish(m_score >= kPointsToWin ? 1 : 0);
            m_state = 3; // game state 3: over
        }
    }

    void HitMouseGame::step(std::uint64_t nowMs) {
        if (!m_begun) {
            return;
        }
        tickTimer(nowMs);
        setSpeeds();
        spawn();
        animate(nowMs);
        updateHammer(nowMs);
        runCountdown(nowMs);
        checkEnd(nowMs);
    }

    bool HitMouseGame::hits(const Hole& hole, core::Point at) const {
        const int lift = m_look == Look::Turtle ? 20 : 0; // turtle sprite sits 20 pixels higher
        const int frame = hole.animFrame;
        int left = 0;
        int right = 0;
        int top = 0;
        int bottom = 0;
        if (!hole.bomb) {
            // target hit box in pixels relative to the hole: top per frame, x edges for frame 4+ or lower, sprite offsets 16 and 42, bottom 90
            constexpr int kTop[] = {0, 64, 56, 50, 28, 14}; // top edge of the target sprite per animation frame, px
            left = frame >= 4 ? 10 + 16 : 17 + 16; // left edge: sprite x 10 (frame 4+) or 17, plus 16 offset, px
            right = frame >= 4 ? 67 + 16 : 59 + 16; // right edge: sprite x 67 (frame 4+) or 59, plus 16 offset, px
            top = kTop[frame] - 42 - lift; // minus the 42 px sprite offset
            bottom = 90 - 42 - lift; // sprite bottom 90 minus the 42 px offset
        } else {
            // bomb hit box in pixels relative to the hole: top per frame, x edges for frame 1 or later, sprite offsets 30 and 51, bottom 99
            constexpr int kTop[] = {0, 83, 58, 39, 33, 28}; // top edge of the bomb sprite per animation frame, px
            left = frame == 1 ? 8 + 30 : 0 + 30; // left edge: sprite x 8 (frame 1) or 0, plus 30 offset, px
            right = frame == 1 ? 49 + 30 : 58 + 30; // right edge: sprite x 49 (frame 1) or 58, plus 30 offset, px
            top = kTop[frame] - 51; // minus the 51 px sprite offset
            bottom = 99 - 51; // sprite bottom 99 minus the 51 px offset
        }
        return hole.position.x + left <= at.x && hole.position.x + right >= at.x && hole.position.y + top <= at.y && hole.position.y + bottom >= at.y;
    }

    HitMouseGame::Strike HitMouseGame::released(core::Point at, std::uint64_t nowMs) {
        Strike strike;
        if (m_state != 1 || m_hammer != Hammer::Ready) { // game state 1: playing
            return strike;
        }
        m_hammerMs = nowMs;
        m_hammer = Hammer::Down;
        for (int index = 1; index <= kHoles; ++index) {
            Hole& hole = m_holes[static_cast<std::size_t>(index)];
            // only holes in states 1 to 3 (rising, going down, up) with a visible frame can be hit
            if (hole.state < 1 || hole.state > 3 || hole.animFrame < 1 || hole.animFrame > kTargetFrames || !hits(hole, at)) {
                continue;
            }
            if (!hole.bomb) {
                hole.state = 4; // hole state 4: hit
                hole.hitFrame = kTargetFrames + 1 - hole.animFrame; // hit animation starts mirrored from the current frame
                hole.animFrame = 0;
                ++m_score;
                m_hooks.playSound("sound/wav1611.wav");
                strike = {Strike::Kind::Target, {at.x, at.y + 60}}; // the light starts 60 pixels below the hit
            } else {
                hole.state = 0;
                hole.animFrame = 0;
                m_score = m_score > kBombPenalty ? m_score - kBombPenalty : 0;
                m_hooks.playSound("sound/SEB0008.wav");
                hole.tickMs = nowMs;
                strike = {
                    Strike::Kind::Bomb,
                    {hole.position.x + 60, hole.position.y + 50}
                }; // the light starts at the bomb's centre: 60 right, 50 down of the hole
            }
            return strike;
        }
        return strike;
    }

    void HitMouseGame::leave() { finish(0); }

    void HitMouseGame::finish(int result) {
        m_hooks.stopMusic();
        m_hooks.finishGame(result);
    }

}
