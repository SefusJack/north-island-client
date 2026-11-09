#include "client/subsystems/minigames/ThrowGame.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kLives = 3; // hearts per thrower
        constexpr int kAiDelayStart = 1000; // initial computer-thrower delay, ms
        constexpr int kOwnLeft = 500; // own half of the field: left edge x, px
        constexpr int kOwnRight = 800; // own half of the field: right edge x, px
        constexpr int kRivalRight = 300; // rival half of the field: right edge x, px (left edge is 0)
        constexpr int kAreaTop = 430; // top edge y of the playing area, px
        constexpr int kAreaBottom = 550; // bottom edge y of the playing area, px
        constexpr int kFirstOwnX = 600; // start x of the first own thrower, px
        constexpr int kFirstRivalX = 200; // start x of the first rival thrower, px
        constexpr int kFirstY = 450; // start y of the first thrower, px
        constexpr int kSpread = 25; // step in x and y between the starting throwers, px
        constexpr int kOwnStanding = 16; // direction/action index: own thrower standing
        constexpr int kRivalStanding = 17; // direction/action index: rival thrower standing
        constexpr int kOwnWindUp = 34; // direction/action index: own thrower winding up
        constexpr int kRivalWindUp = 35; // direction/action index: rival thrower winding up
        constexpr int kRunningLeft = 64; // direction/action index: running left
        constexpr int kRunningRight = 65; // direction/action index: running right
        constexpr int kOwnHit = 30; // direction/action index: own thrower hit by a ball
        constexpr int kRivalHit = 31; // direction/action index: rival thrower hit by a ball
        constexpr int kOwnDown = 26; // direction/action index: own thrower knocked down
        constexpr int kRivalDown = 27; // direction/action index: rival thrower knocked down
        constexpr int kLastWindUpFrame = 3; // wind-up animation frame at which the ball is released
        constexpr int kMostPower = 15; // maximum throw strength
        constexpr int kSlowChargeMs = 60; // time per strength point up to kFastChargeFrom, ms
        constexpr int kFastChargeMs = 30; // time per strength point above kFastChargeFrom, ms
        constexpr int kFastChargeFrom = 8; // strength above which charging speeds up
        constexpr int kRecoveryMs = 100; // time between wind-up frame steps while recovering after a throw, ms
        constexpr int kHitMs = 1000; // time a thrower stays in the hit state, ms
        constexpr int kThrowWindUpMs = 1000; // hold time of a computer thrower's wind-up state, ms
        constexpr int kGameOverMs = 3000; // time the end screen is shown, ms
        constexpr double kWalkSpeed = 0.3; // thrower walk speed, px per ms
        constexpr double kBallSpeed = 0.5; // ball flight speed, px per ms
        constexpr int kPlayerHalfWidth = 20; // half width of a thrower's click/hit box, px
        constexpr int kPlayerHeight = 90; // height of a thrower's click box above the feet, px
        constexpr int kPlayerGrabHeight = 30; // height cut from the top of the click box, px
        constexpr int kBallOffsetX = 30; // ball start x offset from the thrower's feet, px
        constexpr int kBallOffsetY = 50; // ball start y offset above the thrower's feet, px
        constexpr int kHitEffectFrames = 7; // frames in the hit light effect (S10203)
        constexpr int kLandEffectFrames = 12; // frames in the landing light effect (S10004)
        constexpr std::uint64_t kEffectFrameMs = 70; // time per light effect frame, ms
        constexpr int kEffectAlpha = 30; // light effect alpha (0 to 255)
        constexpr int kHitEffectDx = 10; // light effect x offset from the ball, px
        constexpr int kLandEffectDy = 100; // landing effect y offset below the ball, px
        const char* const kMusic = "BGM0018";

        /// @brief Gives how far each point of throw strength carries.
        /// @details Original: AimProjectile's PowerStep.
        /// @param power Throw strength (0 to 15).
        /// @return Pixels per point of strength.
        int powerStep(int power) { return power <= 5 ? 15 : power <= 10 ? 25 : 40; } // strength bands 5/10 and pixels per point 15/25/40

        /// @brief Rounds a double to the nearest int.
        /// @param value Value to round.
        /// @return Nearest integer.
        int rounded(double value) { return static_cast<int>(std::nearbyint(value)); }

    }

    ThrowGame::ThrowGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void ThrowGame::open(int npcId, std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_nowMs = nowMs;
        m_begun = false;
        m_musicStarted = false;
        m_playing = true;
        m_finished = false;
        m_gameOver = 0;
        m_mouseHeld = false;
        m_mouseReleased = false;
        m_throwing = false;
        m_power = 0;
        m_aimIndex = 0;
        m_projectileCount = 0;
        m_drawOrder = {};
        m_projectiles = {};
        createTeams(npcId, nowMs);
        m_hooks.stopMusic();
    }

    void ThrowGame::createTeams(int npcId, std::uint64_t nowMs) {
        m_fighterSprite = m_hooks.npcSprite(npcId);
        m_playerSlot = 1;
        m_creatingTeam = true;
        m_own = {};
        m_rival = {};
        m_own.count = ThrowTeam::kMost;
        m_rival.count = ThrowTeam::kMost;
        for (int j = 1; j <= ThrowTeam::kMost; ++j) {
            const auto index = static_cast<std::size_t>(j);
            ThrowThrower& own = m_own.throwers[index];
            own.screen = {kFirstOwnX + (j - 1) * kSpread, kFirstY + (j - 1) * kSpread};
            own.dest = own.screen;
            own.previousScreen = own.screen;
            own.direction = kOwnStanding;
            ThrowThrower& rival = m_rival.throwers[index];
            rival.screen = {kFirstRivalX - (j - 1) * kSpread, kFirstY + (j - 1) * kSpread};
            rival.dest = rival.screen;
            rival.previousScreen = rival.screen;
            rival.direction = kRivalStanding;
            for (ThrowTeam& side : std::array<std::reference_wrapper<ThrowTeam>, 2>{m_own, m_rival}) {
                side.lives[index] = kLives;
                side.aiDelays[index] = kAiDelayStart;
                side.decisionMs[index] = nowMs;
            }
        }
    }

    void ThrowGame::finish(int result) {
        m_finished = true;
        m_hooks.stopMusic();
        m_hooks.finishGame(result);
    }

    void ThrowGame::leave() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    bool ThrowGame::timerElapsed(std::uint64_t& timer, int interval) const {
        if (m_nowMs - timer >= static_cast<std::uint64_t>(interval)) {
            timer = m_nowMs;
            return true;
        }
        return false;
    }

    core::Point ThrowGame::moveToward(
        core::Point current,
        core::Point target,
        int distX,
        int distY,
        std::uint64_t& tickX,
        std::uint64_t& tickY,
        double speed
    ) const {
        int step = rounded(static_cast<double>(m_nowMs - tickX) * speed);
        tickX = m_nowMs;
        int newX = current.x;
        const auto go = [](int from, int to, int by) {
            if (from < to) {
                return std::min(from + by, to);
            }
            return std::max(from - by, to);
        };
        if (distX >= distY) {
            if (current.x != target.x) {
                newX = go(current.x, target.x, step);
            }
        } else if (current.x != target.x) {
            newX = go(current.x, target.x, rounded(distX / (distY + 0.0001) * step)); // 0.0001 avoids dividing by zero
        }
        step = rounded(static_cast<double>(m_nowMs - tickY) * speed);
        tickY = m_nowMs;
        int newY = current.y;
        if (distX >= distY) {
            if (current.y != target.y) {
                newY = go(current.y, target.y, rounded(distY / (distX + 0.0001) * step)); // 0.0001 avoids dividing by zero
            }
        } else if (current.y != target.y) {
            newY = go(current.y, target.y, step);
        }
        return {newX, newY};
    }

    void ThrowGame::pressed(core::Point at, bool left) {
        const auto slot = static_cast<std::size_t>(m_playerSlot);
        if (m_own.states[slot] != 0 || !left) {
            return;
        }
        const ThrowThrower& player = m_own.throwers[slot];
        const bool inArea = at.x >= kOwnLeft && at.x <= kOwnRight && at.y >= kAreaTop && at.y <= kAreaBottom;
        const bool onFigure = !(player.screen.x - kPlayerHalfWidth > at.x || player.screen.x + kPlayerHalfWidth < at.x) &&
                              !(player.screen.y - kPlayerHeight + kPlayerGrabHeight > at.y || player.screen.y < at.y);
        if (onFigure) {
            m_own.states[slot] = 1;
            m_chargeMs = m_nowMs;
            m_power = 0;
        } else {
            m_own.states[slot] = inArea ? 2 : 0; // state 2: sent to walk
        }
        m_mouseHeld = true;
        m_mouseReleased = false;
        m_ownDest = at;
    }

    void ThrowGame::released(core::Point at, bool left) {
        if (m_own.states[static_cast<std::size_t>(m_playerSlot)] != 0) {
            m_mouseHeld = false;
            m_mouseReleased = true;
        } else if (left) {
            m_mouseHeld = false;
            m_mouseReleased = true;
            m_ownDest = at;
        }
    }

    void ThrowGame::chargePower() {
        const auto slot = static_cast<std::size_t>(m_playerSlot);
        if (m_own.lives[slot] == 0 || m_own.states[slot] != 1 || !m_mouseHeld) {
            return;
        }
        ThrowThrower& player = m_own.throwers[slot];
        player.direction = kOwnWindUp;
        player.holdFrame = 0;
        if (timerElapsed(m_chargeMs, m_power > kFastChargeFrom ? kFastChargeMs : kSlowChargeMs)) {
            m_power = std::min(m_power + 1, kMostPower);
        }
    }

    void ThrowGame::handleAction() {
        const auto slot = static_cast<std::size_t>(m_playerSlot);
        if (m_own.lives[slot] == 0) {
            return;
        }
        ThrowThrower& player = m_own.throwers[slot];
        if (m_mouseReleased) {
            if (m_own.states[slot] == 1) {
                player.direction = kOwnWindUp;
                player.holdFrame = HumanFrame::kAnimate;
                if (player.anim.frame == kLastWindUpFrame) {
                    launch(m_playerSlot, 1, m_power);
                    player.holdFrame = kLastWindUpFrame;
                    m_throwing = true;
                    m_power = 0;
                    m_own.states[slot] = 0;
                    m_throwMs = m_nowMs;
                }
            }
        } else if (m_mouseHeld && m_own.states[slot] == 2) { // state 2: sent to walk
            player.dest = m_ownDest;
            calcOwnMoveDistance();
            m_own.states[slot] = 5; // state 5: walking
            m_moveTickX = m_nowMs;
            m_moveTickY = m_nowMs;
        }
    }

    void ThrowGame::playRecovery() {
        const auto slot = static_cast<std::size_t>(m_playerSlot);
        const int state = m_own.states[slot];
        if (state == 2 || state == 5) { // states 2 sent to walk, 5 walking
            m_throwing = false;
            return;
        }
        if (!m_throwing) {
            return;
        }
        ThrowThrower& player = m_own.throwers[slot];
        player.direction = kOwnWindUp;
        if (!timerElapsed(m_throwMs, kRecoveryMs)) {
            return;
        }
        switch (player.anim.frame) {
            case 3: // wind-up frame 3 (arm forward): step back to frame 2
                player.holdFrame = 2;
                break;
            case 2: // wind-up frame 2: step back to frame 1
                player.holdFrame = 1;
                break;
            case 1: // wind-up frame 1: step back to frame 0
                player.holdFrame = 0;
                break;
            case 0: // wind-up frame 0: recovery over, back to standing
                player.direction = kOwnStanding;
                player.holdFrame = HumanFrame::kAnimate;
                m_own.states[slot] = 0;
                m_throwing = false;
                break;
            default: // any other frame: nothing to do
                break;
        }
    }

    void ThrowGame::calcOwnMoveDistance() {
        const ThrowThrower& player = m_own.throwers[static_cast<std::size_t>(m_playerSlot)];
        m_ownDistX = std::abs(player.screen.x - player.dest.x);
        m_ownDistY = std::abs(player.screen.y - player.dest.y);
    }

    bool ThrowGame::ownThrowerArrived() const {
        const ThrowThrower& player = m_own.throwers[static_cast<std::size_t>(m_playerSlot)];
        if (m_ownDistX >= m_ownDistY) {
            return std::abs(player.screen.x - player.dest.x) <= 0;
        }
        return std::abs(player.screen.y - player.dest.y) <= 0;
    }

    void ThrowGame::moveOwnThrower() {
        const auto slot = static_cast<std::size_t>(m_playerSlot);
        if (m_own.states[slot] != 5) { // state 5: walking
            return;
        }
        ThrowThrower& player = m_own.throwers[slot];
        player.screen = moveToward(player.screen, player.dest, m_ownDistX, m_ownDistY, m_moveTickX, m_moveTickY, kWalkSpeed);
        player.direction = player.screen.x <= player.dest.x ? kRunningRight : kRunningLeft;
        player.holdFrame = HumanFrame::kAnimate;
        if (ownThrowerArrived()) {
            m_own.states[slot] = 0;
            player.direction = kOwnStanding;
            player.holdFrame = HumanFrame::kAnimate;
            if (!m_mouseReleased && m_mouseHeld) {
                m_own.states[slot] = 1;
            }
        }
    }

    void ThrowGame::launch(int index, int side, int power) {
        if (!m_playing || m_projectileCount > kProjectiles) {
            return;
        }
        ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(m_aimIndex + 1)];
        const ThrowThrower& thrower = team(side).throwers[static_cast<std::size_t>(index)];
        ball.at = side == 1 ? core::Point{thrower.screen.x - kBallOffsetX, thrower.screen.y - kBallOffsetY}
                            : core::Point{thrower.screen.x + kBallOffsetX, thrower.screen.y - kBallOffsetY};
        ball.previousAt = ball.at;
        ball.previousArcHeight = 0;
        ball.phase = side;
        aim(m_aimIndex + 1, power, side, index);
        ball.active = 1;
        ball.moveTickX = m_nowMs;
        ball.moveTickY = m_nowMs;
        ++m_aimIndex;
        if (m_aimIndex >= kProjectiles) {
            m_aimIndex = 0;
        }
        ++m_projectileCount;
        m_hooks.playSound("sound/Wav1608.Wav");
    }

    void ThrowGame::aim(int projectile, int power, int side, int index) {
        ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(projectile)];
        const int step = powerStep(power);
        const auto aimAtPerson = [&ball](const ThrowThrower& person) { ball.target = person.screen; };
        if (side == 1) {
            if (index == m_playerSlot) {
                // the throw lands 250 + strength*step - 20 px to the left of the hand and 50 px lower
                ball.target = {ball.at.x - 250 - power * step + 20, ball.at.y + 50};
            } else {
                switch (random(3)) { // three equal chances
                    case 0: // machine's choice 0: throw as far as the strength says
                    case 2: // machine's choice 2: throw as far as the strength says
                        // the throw lands 250 + strength*step - 20 px to the left of the hand and 50 px lower
                        ball.target = {ball.at.x - 250 - power * step + 20, ball.at.y + 50};
                        break;
                    default: // machine's choice 1: aim at a person of the rival side
                        aimAtPerson(m_rival.throwers[static_cast<std::size_t>(random(m_rival.count) + 1)]);
                        break;
                }
            }
        } else if (random(3) < 2) { // two chances in three: aim at a person of the player's side
            aimAtPerson(m_own.throwers[static_cast<std::size_t>(random(m_own.count) + 1)]);
        } else {
            // the throw lands 250 + strength*step - 20 px to the right of the hand and 50 px lower
            ball.target = {ball.at.x + 250 + power * step - 20, ball.at.y + 50};
        }
        ball.distX = std::abs(ball.at.x - ball.target.x);
        ball.distY = std::abs(ball.at.y - ball.target.y);
        ball.distance = rounded(std::sqrt(static_cast<double>(ball.distX) * ball.distX + static_cast<double>(ball.distY) * ball.distY));
    }

    void ThrowGame::moveProjectiles() {
        for (int i = 1; i <= kProjectiles; ++i) {
            ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(i)];
            if (ball.active != 1) {
                continue;
            }
            ball.at = moveToward(ball.at, ball.target, ball.distX, ball.distY, ball.moveTickX, ball.moveTickY, kBallSpeed);
            const double dx = ball.target.x - ball.at.x;
            const double dy = ball.target.y - ball.at.y;
            const int remain = rounded(std::sqrt(dx * dx + dy * dy));
            // arc: 360 px peak mid-flight, squeezed by 0.003*arc^2; 0.001 avoids dividing by zero
            int arc = rounded((0.5 - std::abs(remain / (ball.distance + 0.001) - 0.5)) * 360);
            arc = rounded(arc - std::pow(arc, 2) * 0.003);
            ball.arcHeight = rounded(arc * 2);
        }
    }

    void ThrowGame::startLight(ThrowProjectile& projectile, const char* picture, int frames, core::Point at) {
        LightEffect::Setup setup;
        setup.picture = m_hooks.picture(picture);
        setup.frameMs = kEffectFrameMs;
        setup.firstFrame = 1;
        setup.lastFrame = frames;
        setup.frameCount = frames;
        setup.motion = LightEffect::Motion::Once;
        setup.loop = false;
        setup.alpha = kEffectAlpha;
        projectile.light.start(setup, at, m_nowMs);
    }

    void ThrowGame::landProjectiles() {
        for (int j = 1; j <= kProjectiles; ++j) {
            ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(j)];
            if (ball.active != 1) {
                continue;
            }
            const bool landed = ball.phase == 1 ? !(ball.at.x > ball.target.x) : ball.phase == 2 && !(ball.at.x < ball.target.x);
            if (!landed) {
                continue;
            }
            m_hooks.playSound("sound/SEB0051.Wav");
            startLight(ball, "S10004", kLandEffectFrames, {ball.at.x + kHitEffectDx, ball.at.y + kLandEffectDy});
            ball.at = {};
            ball.target = {};
            ball.active = 3; // ball state 3: has landed
            if (m_projectileCount > 0) {
                --m_projectileCount;
            }
        }
    }

    bool ThrowGame::hits(int projectile, int index, int side) const {
        const ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(projectile)];
        const ThrowThrower& thrower = (side == 1 ? m_rival : m_own).throwers[static_cast<std::size_t>(index)];
        // hit box tolerances in px: ball target vs thrower position (dest*), ball current position vs thrower (cur*)
        const int destYLo = 15; // target may be this far above the thrower, px
        const int destYHi = 15; // target may be this far below the thrower, px
        const int destXLo = side == 1 ? 20 : 70; // target may be this far left of the thrower, px
        const int destXHi = side == 1 ? 70 : 20; // target may be this far right of the thrower, px
        const int curYLo = 70; // ball may be this far above the thrower's feet, px
        const int curYHi = 15; // ball may be this far below the thrower's feet, px
        const int curXLo = 20; // ball may be this far left of the thrower, px
        const int curXHi = 20; // ball may be this far right of the thrower, px
        return ball.target.y - destYLo <= thrower.screen.y && ball.target.y + destYHi >= thrower.screen.y && ball.target.x - destXLo <= thrower.screen.x &&
               ball.target.x + destXHi >= thrower.screen.x && ball.at.x >= thrower.screen.x - curXLo && ball.at.x <= thrower.screen.x + curXHi &&
               ball.at.y >= thrower.screen.y - curYLo && ball.at.y <= thrower.screen.y + curYHi;
    }

    void ThrowGame::checkHits() {
        for (int i = 1; i <= kProjectiles; ++i) {
            ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(i)];
            if (ball.active != 1) {
                continue;
            }
            const int side = ball.phase == 1 ? 2 : 1;
            if (ball.phase != 1 && ball.phase != 2) {
                continue;
            }
            ThrowTeam& target = team(side);
            for (int j = 1; j <= target.count; ++j) {
                const auto index = static_cast<std::size_t>(j);
                if (target.states[index] == 3 || target.lives[index] == 0 || !hits(i, j, ball.phase)) { // state 3: hit
                    continue;
                }
                target.throwers[index].direction = side == 2 ? kRivalHit : kOwnHit;
                target.states[index] = 3; // state 3: hit
                target.hitMs[index] = m_nowMs;
                --target.lives[index];
                ball.active = 2; // ball state 2: has hit a person
                startLight(ball, "S10203", kHitEffectFrames, {ball.at.x + kHitEffectDx, ball.at.y});
                if (target.lives[index] == 0) {
                    target.throwers[index].direction = side == 2 ? kRivalDown : kOwnDown;
                    target.throwers[index].holdFrame = HumanFrame::kAnimate;
                }
                if (m_projectileCount > 0) {
                    --m_projectileCount;
                }
                m_hooks.playSound("sound/SEB0036.Wav");
            }
        }
    }

    void ThrowGame::recoverHitThrowers() {
        for (int i = 1; i <= m_own.count; ++i) {
            const auto index = static_cast<std::size_t>(i);
            if (m_own.states[index] == 3 && timerElapsed(m_own.hitMs[index], kHitMs)) { // state 3: hit
                m_own.throwers[index].direction = kOwnStanding;
                m_own.throwers[index].holdFrame = HumanFrame::kAnimate;
                m_own.states[index] = 0;
            }
        }
    }

    void ThrowGame::aiRecover(int index, int side) {
        ThrowTeam& t = team(side);
        const auto i = static_cast<std::size_t>(index);
        if (timerElapsed(t.hitMs[i], kHitMs)) {
            t.states[i] = 0;
            t.decisionMs[i] = m_nowMs;
            t.aiDelays[i] = random(500) + 500; // next decision after 500 to 999 ms
            t.throwers[i].direction = side == 1 ? kOwnStanding : kRivalStanding;
        }
    }

    void ThrowGame::aiThrowStep(int index, int side) {
        ThrowTeam& t = team(side);
        const auto i = static_cast<std::size_t>(index);
        ThrowThrower& thrower = t.throwers[i];
        const int windUp = side == 1 ? kOwnWindUp : kRivalWindUp;
        switch (t.states[i]) {
            case 0: // state 0 standing: start winding up
                thrower.direction = windUp;
                t.states[i] = 1; // state 1: winding up
                t.throwMs[i] = m_nowMs;
                thrower.holdFrame = 0;
                break;
            case 1: // state 1 winding up: after the hold time let the wind-up run on
                if (timerElapsed(t.throwMs[i], kThrowWindUpMs)) {
                    t.states[i] = 6; // state 6: about to throw
                    thrower.direction = windUp;
                    thrower.holdFrame = HumanFrame::kAnimate;
                }
                break;
            case 6: // state 6 about to throw: throw at the last wind-up frame
                if (thrower.anim.frame == kLastWindUpFrame) {
                    launch(index, side, random(16)); // random strength 0 to 15
                    thrower.direction = side == 1 ? kOwnStanding : kRivalStanding;
                    thrower.holdFrame = HumanFrame::kAnimate;
                    t.states[i] = 0;
                    t.aiDelays[i] = random(1000) + 1000; // next decision after 1000 to 1999 ms
                    t.decisionMs[i] = m_nowMs;
                }
                break;
            default: // any other state: nothing to do
                break;
        }
    }

    void ThrowGame::calcAiMoveDistance(int index, int side) {
        ThrowTeam& t = team(side);
        const auto i = static_cast<std::size_t>(index);
        t.distX[i] = std::abs(t.throwers[i].screen.x - t.throwers[i].dest.x);
        t.distY[i] = std::abs(t.throwers[i].screen.y - t.throwers[i].dest.y);
    }

    bool ThrowGame::aiArrived(int index, int side) const {
        const ThrowTeam& t = team(side);
        const auto i = static_cast<std::size_t>(index);
        if (t.distX[i] >= t.distY[i]) {
            return std::abs(t.throwers[i].screen.x - t.throwers[i].dest.x) <= 0;
        }
        return std::abs(t.throwers[i].screen.y - t.throwers[i].dest.y) <= 0;
    }

    void ThrowGame::aiWalkStep(int index, int side) {
        ThrowTeam& t = team(side);
        const auto i = static_cast<std::size_t>(index);
        ThrowThrower& thrower = t.throwers[i];
        if (t.states[i] == 0) {
            if (side == 1) {
                thrower.dest.x = random(kOwnRight - kOwnLeft) + kOwnLeft;
            } else {
                thrower.dest.x = random(kRivalRight);
            }
            thrower.dest.y = std::max(random(kAreaBottom - kAreaTop) + kAreaTop, kAreaTop);
            calcAiMoveDistance(index, side);
            t.states[i] = 2; // state 2: sent to walk
            t.moveTickX[i] = m_nowMs;
            t.moveTickY[i] = m_nowMs;
        } else if (t.states[i] == 2) { // state 2: sent to walk
            thrower.screen = moveToward(thrower.screen, thrower.dest, t.distX[i], t.distY[i], t.moveTickX[i], t.moveTickY[i], kWalkSpeed);
            thrower.direction = thrower.screen.x <= thrower.dest.x ? kRunningRight : kRunningLeft;
            thrower.holdFrame = HumanFrame::kAnimate;
            if (aiArrived(index, side)) {
                t.states[i] = 0;
                thrower.direction = side == 1 ? kOwnStanding : kRivalStanding;
                t.aiDelays[i] = random(1000) + 1000; // next decision after 1000 to 1999 ms
                t.decisionMs[i] = m_nowMs;
                thrower.holdFrame = HumanFrame::kAnimate;
            }
        }
    }

    void ThrowGame::updateSideAi(int side) {
        ThrowTeam& t = team(side);
        for (int i = 1; i <= t.count; ++i) {
            const auto index = static_cast<std::size_t>(i);
            if ((side == 1 && m_playerSlot == i) || t.lives[index] == 0 || t.throwers[index].throwSide == 0) {
                continue;
            }
            const int state = t.states[index];
            if (state == 3) { // state 3: hit
                aiRecover(i, side);
                continue;
            }
            int mode = t.modes[index];
            if (state == 0) {
                // a new mode 0 to 6 when the decision time is up, else mode 5 (stand)
                t.modes[index] = timerElapsed(t.decisionMs[index], t.aiDelays[index]) ? random(7) : 5;
                mode = t.modes[index];
            }
            if (mode == 1 || mode == 6) { // modes 1 and 6: throw
                aiThrowStep(i, side);
            } else if (mode == 2 || mode == 3) { // modes 2 and 3: walk
                aiWalkStep(i, side);
            }
        }
    }

    void ThrowGame::sortDrawOrder() {
        std::array<int, kDrawItems + 1> sortY{};
        std::array<int, kDrawItems + 1> order{};
        // draw items 1 to 4 are the player's side, 5 to 8 the rival's, 9 up the balls
        for (int i = 1; i <= 4; ++i) {
            sortY[static_cast<std::size_t>(i)] = m_own.throwers[static_cast<std::size_t>(i)].screen.y;
            order[static_cast<std::size_t>(i)] = i;
        }
        for (int i = 5; i <= 8; ++i) {
            sortY[static_cast<std::size_t>(i)] = m_rival.throwers[static_cast<std::size_t>(i - 4)].screen.y;
            order[static_cast<std::size_t>(i)] = i;
        }
        for (int i = 9; i <= kDrawItems; ++i) { // first ball item
            sortY[static_cast<std::size_t>(i)] = m_projectiles[static_cast<std::size_t>(i - 8)].target.y;
            order[static_cast<std::size_t>(i)] = i;
        }
        for (int i = 1; i <= kDrawItems; ++i) {
            for (int j = i + 1; j <= kDrawItems; ++j) {
                if (sortY[static_cast<std::size_t>(i)] > sortY[static_cast<std::size_t>(j)]) {
                    std::swap(sortY[static_cast<std::size_t>(i)], sortY[static_cast<std::size_t>(j)]);
                    std::swap(order[static_cast<std::size_t>(i)], order[static_cast<std::size_t>(j)]);
                }
            }
        }
        m_drawOrder = order;
    }

    void ThrowGame::checkGameOver() {
        if (!m_playing) {
            if (timerElapsed(m_gameOverMs, kGameOverMs)) {
                finish(m_gameOver);
            }
            return;
        }
        if (m_own.lives[static_cast<std::size_t>(m_playerSlot)] == 0) {
            m_gameOverMs = m_nowMs;
            m_playing = false;
            m_gameOver = 0;
            return;
        }
        bool over = false;
        // four people on each side
        for (int i = 1; i <= 4; ++i) {
            if (m_own.lives[static_cast<std::size_t>(i)] != 0) {
                over = false;
                break;
            }
            m_gameOver = 0;
            over = true;
        }
        for (int i = 1; i <= 4; ++i) {
            if (m_rival.lives[static_cast<std::size_t>(i)] != 0) {
                over = false;
                break;
            }
            m_gameOver = 1;
            over = true;
        }
        if (over) {
            m_gameOverMs = m_nowMs;
            m_playing = false;
        }
    }

    void ThrowGame::finishLight(int projectile) {
        ThrowProjectile& ball = m_projectiles[static_cast<std::size_t>(projectile)];
        if (!ball.light.isActive()) {
            ball.active = 0;
        }
    }

    int ThrowGame::frameCountOf(int side, int index, int slot) const {
        return side == 1 && index == m_playerSlot ? m_hooks.playerFrames(slot) : m_hooks.spriteFrames(m_fighterSprite, slot);
    }

    void ThrowGame::drawStep(std::uint64_t nowMs) {
        for (int place = 1; place <= kDrawItems; ++place) {
            const int item = m_drawOrder[static_cast<std::size_t>(place)];
            if (item < 1 || item > 8) { // items 1 to 8 are people, the rest balls
                continue;
            }
            // items 1 to 4 are the player's side, 5 to 8 the rival's
            const int side = item <= 4 ? 1 : 2;
            const int index = item <= 4 ? item : item - 4;
            ThrowTeam& t = team(side);
            const auto i = static_cast<std::size_t>(index);
            ThrowThrower& thrower = t.throwers[i];
            const int down = side == 1 ? kOwnDown : kRivalDown;
            if (t.lives[i] == 0) {
                thrower.direction = down;
            }
            if (thrower.direction == down && thrower.anim.frame == 1) { // hold the lying-down figure on frame 1
                thrower.holdFrame = 1;
            }
            const int frames = frameCountOf(side, index, thrower.direction);
            if (frames > 0) {
                thrower.anim.next(nowMs, frames, thrower.direction == kOwnDown || thrower.direction == kRivalDown, HumanFrame::kIntervalMs, thrower.holdFrame);
            }
        }
    }

    void ThrowGame::frame(std::uint64_t nowMs) {
        m_nowMs = nowMs;
        if (m_begun) {
            if (!m_musicStarted) {
                m_hooks.playMusic(kMusic);
                m_musicStarted = true;
            }
            chargePower();
            handleAction();
            playRecovery();
            moveOwnThrower();
            checkHits();
            updateSideAi(2);
            if (m_creatingTeam) {
                updateSideAi(1);
            }
            recoverHitThrowers();
            moveProjectiles();
            landProjectiles();
            sortDrawOrder();
            checkGameOver();
        }
        if (!m_finished) {
            drawStep(nowMs);
        }
    }

    void ThrowGame::update(std::uint64_t nowMs) {
        constexpr int kMostCatchUp = 10; // most game frames run in one update call
        for (int frames = 0; !m_finished && nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            for (ThrowTeam& side : std::array<std::reference_wrapper<ThrowTeam>, 2>{m_own, m_rival}) {
                for (ThrowThrower& thrower : side.throwers) {
                    thrower.previousScreen = thrower.screen;
                }
            }
            for (ThrowProjectile& ball : m_projectiles) {
                ball.previousAt = ball.at;
                ball.previousArcHeight = ball.arcHeight;
            }
            frame(m_lastFrameMs);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
