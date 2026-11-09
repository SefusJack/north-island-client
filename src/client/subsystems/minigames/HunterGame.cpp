#include "client/subsystems/minigames/HunterGame.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kLives = 3; // lives at the start
        constexpr int kTimeLeft = 30; // game time at the start, seconds
        constexpr int kEndDelay = 100; // frames (30 ms each) between the end and sending the result
        constexpr int kFieldWidth = 800; // width of the area animals walk in, px (screen width)
        constexpr int kFieldTop = 300; // top of the ground area animals walk in, px
        constexpr int kFieldDepth = 300; // height of the ground area animals walk in, px
        constexpr int kBeeTop = 125; // top of the area bees fly in, px
        constexpr int kBeeDepth = 475; // height of the area bees fly in, px
        constexpr int kBeeNpc = 17064; // NPC 17064: Baby Bee
        constexpr int kPigNpc = 17198; // NPC 17198: Pig
        constexpr int kBigPigNpc = 17199; // NPC 17199: Pig (big)
        constexpr int kRabbitNpc = 17036; // NPC 17036: Rabbit (rabbit kinds are this id and its neighbours)
        constexpr int kLife = 50; // life of an ordinary animal, hit points
        constexpr int kPigLife = 250; // life of a pig, hit points
        constexpr int kShotDamage = 50; // damage of one shot, hit points
        constexpr int kLegsToRun = 10; // legs to run: walk-target rounds an animal wanders before it attacks
        constexpr int kAttackSteps = 5; // attack steps before an attack ends
        constexpr int kAttackTickMs = 10; // attack step runs every this many ticks
        constexpr int kSecondTicks = 50; // ticks (30 ms frames) per game second; the time left drops by 1 every this many ticks
        constexpr int kRetargetTicks = 150; // ticks between new walk targets (and legs-to-run steps)
        constexpr int kLowestAttackY = 125; // walk targets with y at or above this are only renewed on retarget ticks, px
        constexpr int kFirstSpawnInterval = 65; // ticks between spawns with one animal on screen
        constexpr int kLeastSpawnInterval = 40; // least ticks between spawns
        constexpr int kKillsToEnd = 50; // kills that end the game
        constexpr int kLifeCounter = 3; // game seconds (kSecondTicks ticks each) a dead animal stays lying before it vanishes
        constexpr core::Point kPlayerAt{400, 560}; // where attacking animals run to (the player), px
        constexpr int kFloorY = 300; // highest y (horizon) a non-rabbit animal may reach; higher ones are clamped to it, px
        constexpr int kStanding = 8; // sprite direction offset for standing (8 + walking direction)
        constexpr int kLying = 26; // sprite direction 26: lying (27 is the mirrored one)
        constexpr int kFacingDown = 4; // sprite direction 4: facing down (towards the player)
        constexpr int kShotFrames = 5; // frames the shot effect lasts
        constexpr int kShakeStep = 15; // screen shake offset, px
        constexpr int kShakeSmall = 6; // small screen shake offset, px
        const char* const kMusic = "BGM0019"; // background music name
        // NPC id of the 7 target kinds: Squirrel, Chameleon, Snake, Snake (twice as likely), Rabbit, Baby Bee, Pig (big)
        constexpr int kTargetNpcs[7] = {17114, 17174, 17186, 17186, 17036, 17064, 17199};

        /// @brief Running pace of an animal kind.
        /// @param npcId NPC table id of the animal.
        /// @return Pixels per millisecond.
        double speedOf(int npcId) {
            switch (npcId) {
                case 17174: // NPC 17174 (Chameleon) pace
                    return 0.12; // pixels per ms
                case 17198: // NPC 17198 (Pig) pace
                case 17199: // NPC 17199 (Pig, big) pace
                    return 0.2; // pixels per ms
                case 17064: // NPC 17064 (bee) pace
                    return 0.22; // pixels per ms
                case 17186: // NPC 17186 (snake) pace
                    return 0.18; // pixels per ms
                case 17036: // NPC 17036 (rabbit) pace
                    return 0.14; // pixels per ms
                default: // any other kind
                    return 0.1; // pixels per ms
            }
        }

        /// @brief Whether an NPC id is one of the rabbit kinds.
        /// @param npcId NPC table id.
        /// @return True for the rabbit id and its neighbours.
        bool isRabbitKind(int npcId) { return npcId >= kRabbitNpc - 1 && npcId <= kRabbitNpc + 1; }

        /// @brief Whether an action slot is a lying one.
        /// @param slot Action slot of the sprite.
        /// @return True for the two lying slots.
        bool isLying(int slot) { return slot == 26 || slot == 27; } // lying action slots

        /// @brief Folds the standing directions onto the walking ones.
        /// @param direction Direction or action slot below 16.
        /// @return The direction 0 to 7.
        /// @details Original: GetDirOffset for the directions of walking and standing.
        int directionOffset(int direction) { return direction < 8 ? direction : direction - 8; } // standing slots start at 8

        /// @brief Rounds to the nearest integer.
        /// @param value Number to round.
        /// @return The rounded value.
        int rounded(double value) { return static_cast<int>(std::nearbyint(value)); }

    }

    HunterGame::HunterGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void HunterGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_roles = {};
        m_roleCount = 0;
        m_started = false;
        m_finished = false;
        m_gameOver = false;
        m_lives = kLives;
        m_timeLeft = kTimeLeft;
        m_endDelay = kEndDelay;
        m_tickCount = 0;
        m_kills = 0;
        m_shaking = false;
        m_shakePhase = 0;
        m_shake = {};
        m_aim = {};
        m_mouse = {};
        m_shot = {};
        m_shotFired = false;
        m_effectFrame = 0;
        m_hooks.stopMusic();
    }

    void HunterGame::begin(std::uint64_t nowMs) {
        m_started = true;
        m_hooks.playMusic(kMusic);
        m_roleCount = 1;
        createRole(1, nowMs);
    }

    void HunterGame::createRole(int index, std::uint64_t nowMs) {
        HunterRole role;
        const int npcId = kTargetNpcs[random(7)]; // number of entries in kTargetNpcs
        role.npc.npcId = npcId;
        if (const auto info = m_hooks.npcInfo(npcId)) {
            role.npc = *info;
        }
        role.maxLife = npcId == kPigNpc || npcId == kBigPigNpc ? kPigLife : kLife;
        role.life = role.maxLife;
        role.frameCounter = kLegsToRun;
        role.deathCounter = kAttackSteps;
        role.pos = {random(kFieldWidth), random(kFieldDepth) + kFieldTop};
        role.screen = role.pos;
        role.dest = {random(kFieldWidth), random(kFieldDepth) + kFieldTop};
        role.moveTickMs = nowMs;
        m_roles[static_cast<std::size_t>(index)] = role;
    }

    void HunterGame::leave() {
        m_hooks.stopMusic();
        m_gameOver = false;
        m_hooks.endGame();
    }

    void HunterGame::finish(int result) {
        m_finished = true;
        m_hooks.finishGame(result);
    }

    void HunterGame::shoot(core::Point at) {
        if (m_gameOver || !m_started) {
            return;
        }
        m_hooks.playSound("sound/SEB0057.wav");
        m_aim = at;
        m_shot = at;
        m_shotFired = true;
        for (int i = 1; i <= m_roleCount; ++i) {
            if (std::optional<HunterRole>& slot = m_roles[static_cast<std::size_t>(i)]; slot && slot->mouseOver) {
                slot->life -= kShotDamage;
            }
        }
    }

    void HunterGame::faceDestination(HunterRole& role) const {
        const int dy = std::abs(role.dest.y - role.pos.y);
        const int dx = std::abs(role.dest.x - role.pos.x);
        const double slope = dy / (dx + 0.001); // avoids division by zero
        const bool leftDown = role.dest.x < role.pos.x && role.dest.y > role.pos.y;
        const bool leftUp = role.dest.x < role.pos.x && role.dest.y < role.pos.y;
        const bool rightDown = role.dest.x > role.pos.x && role.dest.y > role.pos.y;
        const bool rightUp = role.dest.x > role.pos.x && role.dest.y < role.pos.y;
        const bool diagonal = slope > 0.25 && slope < 2.0; // slope band counted as diagonal
        if (leftDown && diagonal) {
            role.direction = 3; // direction 3: down-left
        } else if (leftUp && diagonal) {
            role.direction = 1; // direction 1: up-left
        } else if (rightDown && diagonal) {
            role.direction = 5; // direction 5: down-right
        } else if (rightUp && diagonal) {
            role.direction = 7; // direction 7: up-right
        } else if (role.dest.x > role.pos.x && !(slope > 0.25)) { // 0.25: slope limit for a sideways run
            role.direction = 6; // direction 6: right
        } else if (role.dest.x < role.pos.x && !(slope > 0.25)) { // 0.25: slope limit for a sideways run
            role.direction = 2; // direction 2: left
        } else if (role.dest.y > role.pos.y) {
            role.direction = 4; // direction 4: down
        } else if (role.dest.y < role.pos.y) {
            role.direction = 0;
        }
    }

    void HunterGame::moveAnimal(HunterRole& role, std::uint64_t nowMs) {
        const auto stand = [&role] { role.direction = kStanding + directionOffset(role.direction); };
        if ((role.dest.x == 0 && role.dest.y == 0) || role.pos == role.dest) {
            stand();
            role.moveDone = true;
            return;
        }
        const int elapsed = static_cast<int>(nowMs - role.moveTickMs);
        role.moveTickMs = nowMs;
        const int step = rounded(elapsed * speedOf(role.npc.npcId));
        const double dx = std::abs(role.dest.x - role.pos.x);
        const double dy = std::abs(role.dest.y - role.pos.y);
        const auto settle = [&role] {
            role.screen = role.pos;
            if (role.screen.y > kFloorY || isRabbitKind(role.npc.npcId)) {
                return;
            }
            role.screen.y = kFloorY;
            role.pos.y = kFloorY;
            role.dest.y = kFloorY;
            role.moveDone = true;
        };
        if (step > dx && step > dy) {
            role.pos = role.dest;
            settle();
            return;
        }
        double ratioYX = 0;
        double ratioXY = 0;
        // 0.001 below avoids division by zero
        if (dy > dx) {
            ratioYX = dy / (0.001 + dx);
        } else {
            ratioXY = dx / (0.001 + dy);
        }
        double stepX = step / (ratioYX + 0.001);
        double stepY = step / (ratioXY + 0.001);
        stepX = std::min<double>(stepX, step);
        stepY = std::min<double>(stepY, step);
        int moveX = 0;
        int moveY = 0;
        if (role.pos.x < role.dest.x) {
            moveX = rounded(stepX);
        } else if (role.pos.x > role.dest.x) {
            moveX = -rounded(stepX);
        }
        if (role.pos.y < role.dest.y) {
            moveY = rounded(stepY);
        } else if (role.pos.y > role.dest.y) {
            moveY = -rounded(stepY);
        }
        role.pos.x += moveX;
        role.pos.y += moveY;
        settle();
    }

    void HunterGame::updateShake() {
        static constexpr core::Point kSteps[7] = {
            {kShakeStep, -kShakeStep},
            {-kShakeStep, kShakeStep},
            {kShakeStep, kShakeSmall},
            {-kShakeStep, -kShakeStep},
            {0, 0},
            {kShakeStep, kShakeStep},
            {0, -kShakeSmall}
        };
        m_shake = kSteps[m_shakePhase];
        m_shakePhase = (m_shakePhase + 1) % 7; // number of shake steps
    }

    void HunterGame::spawnTargets(std::uint64_t nowMs) {
        if (m_gameOver) {
            return;
        }
        for (int i = 1; i <= m_roleCount; ++i) {
            std::optional<HunterRole>& slot = m_roles[static_cast<std::size_t>(i)];
            if (!slot || slot->killed || slot->life > 0) {
                continue;
            }
            slot->killed = true;
            slot->lifeCounter = kLifeCounter;
            slot->direction = kLying + (directionOffset(slot->direction) < 4 ? 0 : 1);
            slot->anim.frame = 0;
            ++m_kills;
        }
        for (int i = 1; i <= m_roleCount; ++i) {
            std::optional<HunterRole>& slot = m_roles[static_cast<std::size_t>(i)];
            if (!slot) {
                continue;
            }
            HunterRole& role = *slot;
            if (!role.moveDone && m_tickCount % kRetargetTicks != 0 && role.dest.y >= kLowestAttackY) {
                continue;
            }
            if (!role.attacking) {
                --role.frameCounter;
            }
            if (role.frameCounter == 0) {
                role.attacking = true;
                role.dest = kPlayerAt;
                role.moveDone = false;
                role.moveTickMs = nowMs;
                role.frameCounter = kLegsToRun;
                m_shaking = true;
            }
            if (role.attacking) {
                continue;
            }
            role.dest = {random(kFieldWidth), random(kFieldDepth) + kFieldTop};
            role.moveDone = false;
            role.moveTickMs = nowMs;
            if (role.npc.npcId == kBeeNpc) {
                role.dest = {random(kFieldWidth), random(kBeeDepth) + kBeeTop};
            }
        }
        if (m_tickCount % kSecondTicks == 0) {
            for (int i = 1; i <= m_roleCount; ++i) {
                std::optional<HunterRole>& slot = m_roles[static_cast<std::size_t>(i)];
                if (slot && slot->killed && --slot->lifeCounter == 0) {
                    slot.reset();
                }
            }
        }
        const int interval = std::max(kFirstSpawnInterval - m_roleCount, kLeastSpawnInterval);
        if (m_tickCount % interval == 0 && m_roleCount < kMostRoles) {
            ++m_roleCount;
            createRole(m_roleCount, nowMs);
        }
    }

    void HunterGame::updateStep(std::uint64_t nowMs) {
        for (int i = 1; i <= m_roleCount; ++i) {
            const std::optional<HunterRole>& role = m_roles[static_cast<std::size_t>(i)];
            if (role && role->moveDone && role->attacking) {
                updateShake();
            }
        }
        if (!m_started) {
            return;
        }
        if (m_lives <= 0) {
            m_gameOver = true;
        }
        if (m_kills >= kKillsToEnd || m_timeLeft <= 0) {
            m_gameOver = true;
        }
        if (m_gameOver) {
            --m_endDelay;
            if (m_endDelay <= 0) {
                m_gameOver = false;
                finish(m_timeLeft <= 0 ? 1 : 0);
            }
        }
        ++m_tickCount;
        spawnTargets(nowMs);
        if (m_tickCount % kSecondTicks != 0 || m_gameOver) {
            return;
        }
        --m_timeLeft;
    }

    void HunterGame::drawAnimals(std::uint64_t nowMs) {
        if (m_timeLeft < 0) {
            m_timeLeft = 0;
        }
        const core::Point shake = this->shake();
        for (int i = 1; i <= m_roleCount; ++i) {
            std::optional<HunterRole>& slot = m_roles[static_cast<std::size_t>(i)];
            if (!slot) {
                continue;
            }
            HunterRole& role = *slot;
            role.previousScreen = role.screen;
            if (!role.killed && !m_gameOver) {
                faceDestination(role);
                moveAnimal(role, nowMs);
            }
            role.drawnSlot = role.direction;
            role.drawnScale = role.drawScale;
            role.drawnHeight = m_hooks.spriteSize(role.npc, role.direction).height;
            role.drawnAt = {role.screen.x + shake.x, role.screen.y + shake.y};
            role.drawnFrame = role.anim.next(
                nowMs,
                m_hooks.frameCount(role.npc, role.direction),
                isLying(role.direction),
                role.moveDone ? HumanFrame::kIntervalMs : HumanFrame::kMovingIntervalMs
            );
            role.mouseOver = m_hooks.hit(role.npc, role.drawnAt, role.drawnSlot, role.drawnFrame, role.drawnScale, m_mouse);
            role.attackShown = false;
            if (!role.moveDone || !role.attacking) {
                continue;
            }
            role.attackShown = true;
            role.drawScale = 20; // twice its own size
            if (!role.killed) {
                role.direction = kFacingDown;
            }
            if (m_tickCount % kAttackTickMs != 0) {
                continue;
            }
            if (role.deathCounter == kAttackSteps) {
                --m_lives;
                switch (m_hooks.playerBodyType()) {
                    case 1: // body type 1 hurt cry
                        m_hooks.playSound("sound/SEB0173.wav");
                        break;
                    case 2: // body type 2 hurt cry
                        m_hooks.playSound("sound/SEB0221.wav");
                        break;
                    case 3: // body type 3 hurt cry
                        m_hooks.playSound("sound/SEB0156.wav");
                        break;
                    case 4: // body type 4 hurt cry
                        m_hooks.playSound("sound/SEB0199.wav");
                        break;
                    default: // no cry
                        break;
                }
            }
            --role.deathCounter;
            if (role.deathCounter != 0) {
                continue;
            }
            role.drawScale = 10; // its own size
            role.attacking = false;
            role.deathCounter = kAttackSteps;
            role.frameCounter = kLegsToRun;
            m_shaking = false;
        }
    }

    void HunterGame::drawCrosshair() {
        if (m_shotFired) {
            ++m_effectFrame;
            if (m_effectFrame == kShotFrames) {
                m_shotFired = false;
                m_effectFrame = 0;
            }
        }
    }

    void HunterGame::drawStep(std::uint64_t nowMs) {
        drawAnimals(nowMs);
        drawCrosshair();
    }

    void HunterGame::update(std::uint64_t nowMs, core::Point mouse) {
        constexpr int kMostCatchUp = 10; // most frames run in one update, to avoid a spiral after a stall
        m_mouse = mouse;
        for (int frames = 0; !m_finished && nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            updateStep(m_lastFrameMs);
            drawStep(m_lastFrameMs);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
