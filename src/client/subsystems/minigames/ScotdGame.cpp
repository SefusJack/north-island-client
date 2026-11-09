#include "client/subsystems/minigames/ScotdGame.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <initializer_list>

namespace client {

    namespace {

        constexpr int kStartX = 150; // player start x, px
        constexpr int kEdge = 90; // margin from the left/right field edge that actors cannot pass, px
        constexpr int kStepX = 12; // px moved per walking step
        constexpr int kJumpPower = 50; // jump power at the start of a jump (ticks of the jump arc)
        constexpr int kSpawnJumpPower = 20; // jump power of a newly spawned monster (a short hop in)
        constexpr int kSpawnEvery = 50; // ticks between monster spawns and monster decisions
        constexpr int kEndTicks = 100; // ticks the end state shows before the game finishes
        constexpr int kSwingTicks = 15; // ticks a swing lasts
        constexpr double kMoveSpeed = 0.16; // movement speed, px per ms
        constexpr int kLyingLeft = 26; // action slot 26: lying, facing left
        constexpr int kLyingRight = 27; // action slot 27: lying, facing right
        constexpr int kHitAction = 30; // action 30: player hurt (slots 30/31 left/right)
        constexpr int kStandAction = 8; // action 8: standing (8-direction group)

        /// @brief Tells whether a value is one of the listed values.
        /// @param value The value to look for.
        /// @param values The candidates.
        /// @return True when found.
        bool oneOf(int value, std::initializer_list<int> values) { return std::find(values.begin(), values.end(), value) != values.end(); }

        /// @brief Offset of a direction slot inside its action group.
        /// @details Original: GetDirOffset, for the directions this game gives (walking, standing and its own actions).
        /// @param direction Action slot of the sprite.
        /// @return Offset from the first slot of the group (0 when in none).
        int directionOffset(int direction) {
            /// @brief A range of sprite action slots forming one action group.
            struct Group {
                int first;
                int last;
            };

            // action slot groups {first, last} of the sprites
            constexpr Group groups[] = {
                {0, 7},
                {8, 15},
                {16, 17},
                {18, 25},
                {26, 27},
                {28, 29},
                {30, 31},
                {36, 37},
                {34, 35},
                {32, 33},
                {38, 39},
                {40, 41},
                {42, 43},
                {44, 45},
                {46, 53},
                {54, 61},
                {66, 73},
                {62, 63},
                {64, 65}
            };
            for (const Group& group : groups) {
                if (direction >= group.first && direction <= group.last) {
                    return direction - group.first;
                }
            }
            return 0;
        }

        /// @brief Tells whether an action has eight directions.
        /// @details Original: IsEightDirAction.
        /// @param action The action.
        /// @return True for the eight-direction actions.
        bool eightDirection(int action) { return oneOf(action, {0, 8, 18, 46, 54, 66}); } // action ids that have eight directions

    }

    ScotdGame::ScotdGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void ScotdGame::open(std::uint64_t nowMs, int hearts) {
        m_lastFrameMs = nowMs;
        m_remaining = hearts;
        m_player = ScotdRole{};
        m_player.pos = {kStartX, kGroundY};
        m_player.screen = m_player.pos;
        m_player.dest = m_player.pos;
        m_player.moveTickMs = nowMs;
    }

    void ScotdGame::finish(int result) {
        m_over = true;
        m_hooks.stopMusic();
        m_hooks.finishGame(result);
    }

    void ScotdGame::leave() { finish(0); }

    void ScotdGame::begin() {
        m_started = true;
        m_hooks.playMusic("BGM0015"); // music id of the Scotd play
    }

    void ScotdGame::updateMove(ScotdRole& role, std::uint64_t nowMs) const {
        const std::uint64_t elapsed = nowMs - role.moveTickMs;
        role.moveTickMs = nowMs;
        const int speed = static_cast<int>(std::nearbyint(kMoveSpeed * static_cast<double>(elapsed)));
        role.screen = role.pos;
        if (std::abs(role.dest.x - role.pos.x) <= speed && std::abs(role.dest.y - role.pos.y) <= speed) {
            role.pos = role.dest;
            role.screen = role.pos;
        }
    }

    void ScotdGame::setAction(ScotdRole& role, int action) const {
        int offset = 0;
        if (eightDirection(role.curAction) == eightDirection(action)) {
            offset = directionOffset(role.direction);
        }
        if (eightDirection(role.curAction) && !eightDirection(action)) {
            offset = directionOffset(role.direction) <= 3 ? 0 : 1; // offsets 0..3 face left (0), others right (1)
        }
        if (!eightDirection(role.curAction) && eightDirection(action)) {
            offset = directionOffset(role.direction) == 0 ? 1 : 5; // eight-direction offset of the left (1) or right (5) facing
        }
        // 26: the lying-left action slot
        if (role.curAction == 26) {
            role.holdFrame = HumanFrame::kAnimate;
        }
        if (action == 26) {
            role.anim.frame = 0;
        }
        role.curAction = action;
        role.direction = action + offset;
    }

    void ScotdGame::stepShift(ScotdRole& role) const {
        if (!role.inJump) {
            return;
        }
        role.moving = false;
        role.moveSteps = 0;
        // the jump moves 5 pixels a tick, up while the power is above 25, and shows frame 0..3 by the power ranges 30-50, 25-29, 20-24, 1-19
        role.pos.y += role.jumpPower > 25 ? -5 : 5;
        if (role.jumpPower >= 30 && role.jumpPower <= 50) {
            role.holdFrame = 0;
        } else if (role.jumpPower >= 25 && role.jumpPower <= 29) {
            role.holdFrame = 1;
        } else if (role.jumpPower >= 20 && role.jumpPower <= 24) {
            role.holdFrame = 2;
        } else if (role.jumpPower >= 1 && role.jumpPower <= 19) {
            role.holdFrame = 3;
        }
        --role.jumpPower;
        if (role.jumpPower == 0) {
            role.inJump = false;
            role.holdFrame = HumanFrame::kAnimate;
            // slots 36/37: jumping left/right; back to the standing slots 2/6 (left/right)
            if (role.direction == 36) {
                role.direction = 2;
            }
            if (role.direction == 37) {
                role.direction = 6;
            }
        }
    }

    void ScotdGame::stepSwing() {
        if (m_remaining == 0 || !m_player.attacking) {
            return;
        }
        --m_player.jumpCounter;
        if (m_player.jumpCounter != 0) {
            return;
        }
        m_player.attacking = false;
        // slots 34/35: swinging left/right; back to the standing slots 2/6; a jumping player goes on to the jump slots 36/37
        if (m_player.direction == 34) {
            m_player.direction = 2;
        }
        if (m_player.direction == 35) {
            m_player.direction = 6;
        }
        if (!m_player.inJump) {
            return;
        }
        if (m_player.direction == 2) {
            m_player.direction = 36;
        }
        if (m_player.direction == 6) {
            m_player.direction = 37;
        }
    }

    void ScotdGame::stepMoveX(ScotdRole& role) const {
        // slots 65/37/35 face right, 64/36/34 face left (walk, jump, swing); 65/64 fall back to standing slots 6/2
        if (role.moving && oneOf(role.direction, {65, 37, 35}) && kFieldWidth - kEdge >= role.pos.x) {
            role.pos.x += kStepX;
            --role.moveSteps;
        }
        if (role.moving && oneOf(role.direction, {64, 36, 34}) && kEdge <= role.pos.x) {
            role.pos.x -= kStepX;
            --role.moveSteps;
        }
        if (role.moveSteps == 0) {
            if (role.direction == 65) {
                role.direction = 6;
            }
            if (role.direction == 64) {
                role.direction = 2;
            }
            role.moving = false;
            role.moveSteps = 0;
        }
    }

    void ScotdGame::jump(ScotdRole& role) const {
        // direction slots: left group {2, 38, 9, 10, 8, 26} -> 36 (jump left), right group {12, 13, 14, 6, 39, 27} -> 37 (jump right); 64/65 walk, 26/27 lying
        if (role.moving) {
            if (role.direction == 64 || role.direction == 26) {
                role.direction = 2;
            }
            if (role.direction == 65 || role.direction == 27) {
                role.direction = 6;
            }
        }
        role.moving = false;
        role.moveSteps = 0;
        if (oneOf(role.direction, {2, 38, 9, 10, 8, 26})) {
            role.direction = 36;
        }
        if (oneOf(role.direction, {12, 13, 14, 6, 39, 27})) {
            role.direction = 37;
        }
        role.dest = {role.pos.x + 10, role.pos.y + 10}; // destination offset in pixels so the move logic does not snap to the position
        role.inJump = true;
        role.jumpPower = kJumpPower;
    }

    void ScotdGame::mouseDown(int button, core::Point at) {
        if (m_remaining == 0) {
            return;
        }
        m_mouseHeld = true;
        if (button == 0) {
            m_click = at;
        }
        if (button != 1 || m_player.inJump) {
            return;
        }
        updateMove(m_player, m_nowMs);
        jump(m_player);
    }

    void ScotdGame::startSwing(std::uint64_t nowMs) {
        // direction slots: left group {2, 38, 9, 10, 8, 36} -> 34 (swing left), right group {12, 13, 14, 6, 39, 37} -> 35 (swing right); 64/65 walk
        if (m_player.moving) {
            if (m_player.direction == 64) {
                m_player.direction = 2;
            }
            if (m_player.direction == 65) {
                m_player.direction = 6;
            }
        }
        updateMove(m_player, nowMs);
        m_player.moving = false;
        m_player.moveSteps = 0;
        m_player.attacking = true;
        m_player.jumpCounter = kSwingTicks;
        if (oneOf(m_player.direction, {2, 38, 9, 10, 8, 36})) {
            m_player.direction = 34;
        }
        if (oneOf(m_player.direction, {12, 13, 14, 6, 39, 37})) {
            m_player.direction = 35;
        }
    }

    void ScotdGame::keyDown(Key key) {
        if (m_remaining == 0) {
            return;
        }
        ScotdRole& player = m_player;
        // a key press walks 5 steps; slot 64 walks left, 65 walks right
        if (key == Key::Left && player.pos.x >= kEdge) {
            if (!player.inJump) {
                player.direction = 64;
            }
            player.moving = true;
            player.moveSteps = 5;
        }
        if (key == Key::Right && kFieldWidth - kEdge >= player.pos.x) {
            if (!player.inJump) {
                player.direction = 65;
            }
            player.moving = true;
            player.moveSteps = 5;
        }
        if (key == Key::Quit) {
            finish(0);
            return;
        }
        if (key == Key::Up && !player.inJump) {
            updateMove(player, m_nowMs);
            jump(player);
        }
        if (key == Key::Space) {
            if (!player.inJump) {
                m_mouseHeld = false;
            }
            if (!player.attacking) {
                startSwing(m_nowMs);
            }
        }
        if (key == Key::Down) {
            // standing slots facing left {2, 38, 9, 10, 8} duck to 26, facing right {12, 13, 14, 6, 39} to 27
            if (oneOf(player.direction, {2, 38, 9, 10, 8})) {
                player.direction = 26;
            }
            if (oneOf(player.direction, {12, 13, 14, 6, 39})) {
                player.direction = 27;
            }
        }
    }

    void ScotdGame::steerToClick(std::uint64_t nowMs) {
        if (m_remaining == 0) {
            return;
        }
        ScotdRole& player = m_player;
        const auto width = [this, &player] { return m_hooks.bodySize(true, player.direction).width; };
        // slot 65 walks right, 64 walks left; 5 steps each
        if (width() + player.screen.x < m_click.x && kFieldWidth - kEdge >= player.pos.x) {
            if (!player.inJump) {
                player.direction = 65;
            }
            player.moving = true;
            player.moveSteps = 5;
        }
        if (player.screen.x - width() > m_click.x && player.pos.x >= kEdge) {
            if (!player.inJump) {
                player.direction = 64;
            }
            player.moving = true;
            player.moveSteps = 5;
        }
        if (width() + player.screen.x > m_click.x && player.screen.x - width() < m_click.x) {
            m_mouseHeld = false;
            if (!player.attacking) {
                startSwing(nowMs);
                switch (m_hooks.playerBodyType()) {
                    case 1: m_hooks.playSound("sound/SEB0027.wav"); break; // body type 1: swing sound
                    case 2: m_hooks.playSound("sound/SEB0018.wav"); break; // body type 2: swing sound
                    case 3: m_hooks.playSound("sound/SEB0033.wav"); break; // body type 3: swing sound
                    case 4: m_hooks.playSound("sound/SEB0039.wav"); break; // body type 4: swing sound
                    default: break; // other body types: no sound
                }
            }
        }
    }

    void ScotdGame::checkCatches() {
        if (m_remaining == 0) {
            return;
        }
        ScotdRole& player = m_player;
        const int height = m_hooks.bodySize(true, player.direction).height;
        const int width = m_hooks.bodySize(true, player.direction).width;
        if (!player.attacking) {
            return;
        }
        // direction slots: facing-left group -> 26 (lying left), facing-right group -> 27 (lying right); 36/37 jumping
        const auto caughtDirection = [](ScotdRole& role) {
            if (oneOf(role.direction, {2, 38, 9, 10, 8, 26, 36})) {
                role.direction = 26;
            }
            if (oneOf(role.direction, {12, 13, 14, 6, 39, 26, 37})) {
                role.direction = 27;
            }
        };
        // slots 2/34: standing/swinging left; 15 is the extra reach in pixels
        if (player.direction == 2 || player.direction == 34) {
            for (int j = 1; j <= m_roleCount; ++j) {
                std::optional<ScotdRole>& slot = m_roles[static_cast<std::size_t>(j)];
                if (!slot || slot->caught || slot->pos.y < player.pos.y - height || player.pos.y + height / 2 < slot->pos.y ||
                    slot->pos.x <= player.pos.x - width - 15 || player.pos.x + width / 2 <= slot->pos.x) {
                    continue;
                }
                ScotdRole& role = *slot;
                m_hooks.playSound("sound/SEB0156.wav"); // catch sound
                role.caught = true;
                caughtDirection(role);
                m_effectPoint.x =
                    (player.screen.x - m_hooks.bodySize(true, player.direction).width + role.screen.x - m_hooks.bodySize(false, role.direction).width) / 2;
                m_effectPoint.y = player.screen.y - m_hooks.bodySize(true, player.direction).height;
                m_effectOn = true;
                role.moving = false;
                role.inJump = false;
                ++m_catchCount;
                if (m_catchCount == 10) { // the tenth catch gives an extra heart
                    ++m_remaining;
                }
            }
        }
        // slots 6/35: standing/swinging right; 15 is the extra reach in pixels
        if (player.direction == 6 || player.direction == 35) {
            for (int j = 1; j <= m_roleCount; ++j) {
                std::optional<ScotdRole>& slot = m_roles[static_cast<std::size_t>(j)];
                if (!slot || slot->caught || slot->pos.y < player.pos.y - height || slot->pos.y > player.pos.y || slot->pos.x <= player.pos.x ||
                    slot->pos.x >= player.pos.x + width + 15) {
                    continue;
                }
                ScotdRole& role = *slot;
                m_hooks.playSound("sound/SEB0156.wav"); // catch sound
                role.caught = true;
                role.moving = false;
                role.inJump = false;
                caughtDirection(role);
                ++m_catchCount;
                m_effectPoint.x = (player.screen.x + role.screen.x) / 2;
                m_effectPoint.y = player.screen.y - m_hooks.bodySize(true, player.direction).height;
                m_effectOn = true;
                if (m_catchCount == 10) { // the tenth catch gives an extra heart
                    ++m_remaining;
                }
            }
        }
    }

    void ScotdGame::checkPlayerHit() {
        ScotdRole& player = m_player;
        const int height = m_hooks.bodySize(true, player.direction).height;
        const int width = m_hooks.bodySize(true, player.direction).width;
        for (int j = 1; j <= m_roleCount; ++j) {
            const std::optional<ScotdRole>& slot = m_roles[static_cast<std::size_t>(j)];
            if (!slot || slot->caught || !(player.pos.x - width / 2 < slot->pos.x) || !(player.pos.x + width / 2 > slot->pos.x) || player.attacking ||
                // 26: the lying action (a lying player cannot be hit)
                player.hitStun || !(player.pos.y + height / 2 > slot->pos.y) || !(player.pos.y - height / 2 < slot->pos.y) || player.curAction == 26) {
                continue;
            }
            --m_remaining;
            setAction(player, kHitAction);
            player.hitStun = true;
            player.hitFlash = true;
            switch (m_hooks.playerBodyType()) {
                case 1: m_hooks.playSound("sound/SEB0173.wav"); break; // body type 1: hurt sound
                case 2: m_hooks.playSound("sound/SEB0221.wav"); break; // body type 2: hurt sound
                case 3: m_hooks.playSound("sound/SEB0156.wav"); break; // body type 3: hurt sound
                case 4: m_hooks.playSound("sound/SEB0199.wav"); break; // body type 4: hurt sound
                default: break; // other body types: no sound
            }
        }
    }

    void ScotdGame::updatePlayer(std::uint64_t nowMs) {
        if (m_remaining == 0) {
            return;
        }
        checkCatches();
        if (m_player.pos.y >= kGroundY) {
            m_player.pos.y = kGroundY;
        }
        updateMove(m_player, nowMs);
        stepMoveX(m_player);
        stepShift(m_player);
        stepSwing();
        if (m_player.hitStun) {
            --m_player.hitStunCounter;
            if (m_player.hitStunCounter == 0) {
                m_player.hitStunCounter = 100; // stun lasts 100 ticks
                m_player.hitStun = false;
            }
        }
        if (m_player.hitFlash) {
            --m_player.hitFlashCounter;
            if (m_player.hitFlashCounter == 0) {
                m_player.hitFlashCounter = 50; // flash lasts 50 ticks
                m_player.hitFlash = false;
            }
        }
    }

    void ScotdGame::spawn(std::uint64_t nowMs) {
        ++m_roleCount;
        ScotdRole role;
        // x spans the field less twice the edge margin (180 = 2 * 90); y starts up to a third of the field height above the ground
        role.pos.x = m_hooks.random(kFieldWidth - 180) + kEdge;
        role.pos.y = kGroundY - m_hooks.random(kFieldHeight / 3);
        role.screen = role.pos;
        role.dest = role.pos;
        role.inJump = true;
        role.jumpPower = kSpawnJumpPower;
        role.moveTickMs = nowMs;
        m_roles[static_cast<std::size_t>(m_roleCount)] = role;
    }

    void ScotdGame::updateRoles(std::uint64_t nowMs) {
        checkPlayerHit();
        if (m_tickCount % kSpawnEvery == 0 && m_roleCount < kMaxRoles) {
            spawn(nowMs);
        }
        for (int j = 1; j <= m_roleCount; ++j) {
            std::optional<ScotdRole>& slot = m_roles[static_cast<std::size_t>(j)];
            if (!slot) {
                continue;
            }
            ScotdRole& role = *slot;
            stepShift(role);
            if (!role.inJump && role.pos.y < kGroundY) {
                role.pos.y += 6; // NPC falls 6 pixels a tick
            }
            if (role.pos.y > kGroundY) {
                role.pos.y = kGroundY;
            }
            if (m_tickCount % kSpawnEvery == 0 && role.pos.y == kGroundY && m_hooks.random(100) > 50 &&
                !role.caught) { // roughly half the chances (random 0..99 above 50) start a jump
                role.inJump = true;
                role.jumpPower = kJumpPower;
            }
            if (m_tickCount % kSpawnEvery == 0) {
                role.animPhase = m_hooks.random(2) + 1; // animPhase 1 walks left, 2 walks right
                if (role.caught) {
                    --role.despawnCountdown;
                }
                if (role.despawnCountdown == 0) {
                    slot.reset();
                }
            }
        }
    }

    void ScotdGame::updateStep(std::uint64_t nowMs) {
        if (!m_started) {
            return;
        }
        if (m_mouseHeld) {
            steerToClick(nowMs);
        }
        ++m_tickCount;
        updatePlayer(nowMs);
        updateRoles(nowMs);
        if (m_remaining != 0 && m_catchCount < kCatchesToWin) {
            return;
        }
        if (m_remaining == 0) {
            // direction slots facing left {2, 38, 9, 10, 8, 30, 36} and right {12, 13, 14, 6, 39, 31, 37} (30/31 hit) fall to the lying slots
            if (oneOf(m_player.direction, {2, 38, 9, 10, 8, 30, 36})) {
                m_player.direction = kLyingLeft;
            }
            if (oneOf(m_player.direction, {12, 13, 14, 6, 39, 31, 37})) {
                m_player.direction = kLyingRight;
            }
            m_player.pos.y = kGroundY;
        }
        ++m_endCounter;
        if (m_endCounter == kEndTicks) {
            m_endCounter = 0;
            if (m_remaining == 0) {
                finish(0);
            } else if (m_catchCount >= kCatchesToWin) {
                finish(1);
            }
        }
    }

    void ScotdGame::drawPlayer(std::uint64_t nowMs) {
        ScotdRole& player = m_player;
        player.drawnSlot = player.direction;
        player.drawnAt = player.screen;
        player.drawnFrame = player.anim.next(
            nowMs,
            m_hooks.frameCount(true, player.direction),
            player.direction == kLyingLeft || player.direction == kLyingRight,
            HumanFrame::kIntervalMs,
            player.holdFrame
        );
        player.shadowAt = player.inJump ? core::Point{player.screen.x, kGroundY} : player.screen;
        if (m_remaining == 0 || player.inJump || player.attacking || player.hitFlash || player.moving || player.direction == kLyingLeft ||
            player.direction == kLyingRight) {
            return;
        }
        setAction(player, kStandAction);
    }

    void ScotdGame::drawRoles(std::uint64_t nowMs) {
        for (int j = 1; j <= m_roleCount; ++j) {
            std::optional<ScotdRole>& slot = m_roles[static_cast<std::size_t>(j)];
            if (!slot) {
                continue;
            }
            ScotdRole& role = *slot;
            if (!role.caught) {
                // walks 3 pixels a draw; phase 2 goes right (standing slot 6), else left (standing slot 2)
                if (role.animPhase == 2) {
                    role.pos.x += 3;
                    role.direction = 6;
                    if (kFieldWidth - kEdge <= role.pos.x) {
                        role.animPhase = 1;
                    }
                } else {
                    role.pos.x -= 3;
                    role.direction = 2;
                    if (role.pos.x <= kEdge) {
                        role.animPhase = 2;
                    }
                }
            }
            updateMove(role, nowMs);
            if (role.inJump) {
                role.shadowAt = {role.screen.x, kGroundY};
                // direction slots: left group -> 36 (jump left), right group -> 37 (jump right)
                if (oneOf(role.direction, {2, 38, 9, 10, 8, 26})) {
                    role.direction = 36;
                }
                if (oneOf(role.direction, {12, 13, 14, 6, 39, 27})) {
                    role.direction = 37;
                }
            } else {
                role.shadowAt = role.screen;
                // direction slots: left group -> 2 (stand left), right group -> 6 (stand right)
                if (oneOf(role.direction, {2, 38, 9, 10, 8, 26, 36})) {
                    role.direction = 2;
                }
                if (oneOf(role.direction, {12, 13, 14, 6, 39, 26, 37})) {
                    role.direction = 6;
                }
            }
            role.drawnSlot = role.direction;
            role.drawnAt = role.screen;
            role.drawnFrame = role.anim.next(
                nowMs,
                m_hooks.frameCount(false, role.direction),
                role.direction == kLyingLeft || role.direction == kLyingRight,
                HumanFrame::kIntervalMs,
                role.holdFrame
            );
        }
    }

    void ScotdGame::drawEffect() {
        m_effectShown = false;
        if (!m_effectOn) {
            return;
        }
        ++m_effectFrame;
        if (m_effectFrame != kEffectFrames + 1) { // frames 1..4 are shown, the fifth ends the effect
            m_effectShown = true;
            m_effectAt = m_effectPoint;
        } else {
            m_effectOn = false;
            m_effectFrame = 0;
        }
    }

    void ScotdGame::drawStep(std::uint64_t nowMs) {
        drawPlayer(nowMs);
        drawRoles(nowMs);
        drawEffect();
    }

    void ScotdGame::update(std::uint64_t nowMs) {
        constexpr int kMostCatchUp = 10; // max frames processed per call when behind
        m_nowMs = nowMs;
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp && !m_over; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_nowMs = m_lastFrameMs;
            m_player.previousDrawnAt = m_player.drawnAt;
            m_player.previousShadowAt = m_player.shadowAt;
            for (std::optional<ScotdRole>& role : m_roles) {
                if (role) {
                    role->previousDrawnAt = role->drawnAt;
                    role->previousShadowAt = role->shadowAt;
                }
            }
            updateStep(m_lastFrameMs);
            if (m_over) {
                return;
            }
            drawStep(m_lastFrameMs);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
