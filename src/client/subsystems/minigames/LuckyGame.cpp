#include "client/subsystems/minigames/LuckyGame.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kThrowerSpriteForTable = 206; // thrower sprite id (plus 1000 below) when the game is started at the table NPC
        constexpr int kThrowerSpriteOtherwise = 483; // thrower sprite id (plus 1000 below) at any other NPC
        constexpr int kTableNpc = 10000; // NPC 10000: Breillat
        constexpr int kThrowerFirstX = 400; // start x of the player and the thrower: screen centre, px
        constexpr int kThrowerLeftLimit = 30; // thrower turns back left of this x, px
        constexpr int kThrowerRightLimit = 770; // thrower turns back right of this x, px
        constexpr int kThrowerStep = 7; // thrower speed, px per frame
        constexpr int kPlayerStep = 10; // player speed, px per frame
        constexpr int kFallLimitY = 550; // objects falling below this y are gone, px
        constexpr int kFallSpeed = 10; // object fall speed, px per frame
        constexpr int kSpawnRise = 50; // objects start this many px above the thrower, px
        constexpr int kCatchSideSlack = 10; // extra px each side of the catch width
        constexpr int kCatchAbove = 50; // catch zone extends this far above the player, px
        constexpr int kCatchBelow = 20; // catch zone extends this far below the player y, px
        constexpr int kLightDrop = 50; // bomb flash is drawn this many px below the bomb, px
        constexpr int kCoin = 1; // object kind 1: coin
        constexpr int kBomb = 2; // object kind 2: bomb
        constexpr int kStanding = 12; // player action slot 12: standing
        constexpr int kWalkingLeft = 2; // player action slot 2: walking left
        constexpr int kWalkingRight = 6; // player action slot 6: walking right
        constexpr int kLyingDown = 27; // player action slot 27: lying down (lost all hearts)
        constexpr int kThrowerLeft = 5; // thrower action slot 5: used when it turns at the left limit
        constexpr int kThrowerRight = 3; // thrower action slot 3: used when it turns at the right limit
        constexpr int kWinScore = 30; // score needed to win, coins
        constexpr int kStartHearts = 3; // hearts (lives) at the start
        constexpr int kStartTime = 30; // game time at the start, seconds
        constexpr int kStartCountdown = 4; // start countdown, seconds before play begins
        constexpr std::uint64_t kSecondMs = 1000; // one second, ms
        constexpr std::uint64_t kOverMs = 3000; // time the game-over screen shows before the result, ms
        constexpr int kDropChances = 15; // drop roll range: 0 and 1 coin, 2 bomb, others nothing
        constexpr int kBombFrames = 2; // animation frames of the bomb picture
        const char* const kCoinPicture = "2302"; // coin picture name
        const char* const kBombPicture = "S10816L"; // bomb picture name
        const char* const kMusic = "BGM0019"; // background music name

        /// @brief Whether an action slot is a lying one.
        /// @param slot Action slot of the sprite.
        /// @return True for the two lying slots.
        bool isLyingGroup(int slot) { return slot == 26 || slot == 27; } // lying action slots

    }

    LuckyGame::LuckyGame(
        Hooks hooks,
        int npcId,
        int lastDropX
    )
        : m_hooks(std::move(hooks)),
          m_lastDropX(lastDropX),
          m_throwerSprite((npcId == kTableNpc ? kThrowerSpriteForTable : kThrowerSpriteOtherwise) + 1000) {}

    void LuckyGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_blinkMs = nowMs;
        m_formShown = false;
        m_phase = Phase::Countdown;
        m_score = 0;
        m_hearts = kStartHearts;
        m_timeLeft = kStartTime;
        m_countdown = kStartCountdown;
        m_playerX = kThrowerFirstX;
        m_throwerX = kThrowerFirstX;
        m_previousPlayerX = m_playerX;
        m_previousThrowerX = m_throwerX;
        m_throwerDirection = 1;
        m_playerSlot = 0;
        m_throwerSlot = kThrowerLeft;
        m_objects = {};
        m_shownFrames = {};
    }

    void LuckyGame::quitClicked() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void LuckyGame::finish(int result) {
        m_hooks.stopMusic();
        m_hooks.finishGame(result);
    }

    void LuckyGame::countdownStep(std::uint64_t nowMs) {
        if (m_phase != Phase::Countdown) {
            return;
        }
        if (nowMs - m_blinkMs >= kSecondMs || m_countdown < 0) {
            if (m_countdown < 0) {
                m_phase = Phase::Playing;
                m_hooks.playMusic(kMusic);
            }
            --m_countdown;
            if (m_countdown > 0) {
                m_hooks.playSound("sound/wav0145.wav");
            }
            if (m_countdown == 0) {
                m_hooks.playSound("sound/wav1610.wav");
            }
            m_blinkMs = nowMs;
        }
    }

    void LuckyGame::moveToMouse(int mouseX) {
        if (m_playerX == mouseX) {
            return;
        }
        if (m_playerX > mouseX) {
            if (m_playerX - kPlayerStep < mouseX) {
                m_playerX = mouseX;
                m_playerSlot = kStanding;
            } else {
                m_playerX -= kPlayerStep;
                m_playerSlot = kWalkingLeft;
            }
        } else if (m_playerX + kPlayerStep > mouseX) {
            m_playerX = mouseX;
            m_playerSlot = kStanding;
        } else {
            m_playerX += kPlayerStep;
            m_playerSlot = kWalkingRight;
        }
    }

    void LuckyGame::bounceThrower() {
        m_throwerX += m_throwerDirection * kThrowerStep;
        if (m_throwerX < kThrowerLeftLimit) {
            m_throwerDirection = 1;
            m_throwerSlot = kThrowerLeft;
        }
        if (m_throwerX > kThrowerRightLimit) {
            m_throwerDirection = -1;
            m_throwerSlot = kThrowerRight;
        }
    }

    void LuckyGame::fallObjects() {
        for (FallingObject& object : m_objects) {
            if (object.inUse) {
                object.y += object.fallSpeed;
                if (object.y > kFallLimitY) {
                    object.inUse = false;
                }
            }
        }
    }

    void LuckyGame::checkCatches() {
        for (FallingObject& object : m_objects) {
            if (!object.inUse || !(m_playerX - object.halfWidth - kCatchSideSlack < object.x) || !(object.x < object.halfWidth + m_playerX + kCatchSideSlack) ||
                !(kPlayerY - object.anchorY - kCatchAbove < object.y) || !(object.y < kPlayerY + kCatchBelow)) {
                continue;
            }
            if (object.kind == kCoin) {
                ++m_score;
                m_hooks.playSound("sound/wav0152.wav");
            } else if (object.kind == kBomb) {
                --m_hearts;
                m_hooks.bombLight({object.x, object.y + kLightDrop});
                m_hooks.playSound("sound/SEB0036.wav");
                if (m_hearts == 0) {
                    m_playerSlot = kLyingDown;
                }
            }
            object.inUse = false;
        }
    }

    void LuckyGame::tickClock(std::uint64_t nowMs) {
        if (m_phase == Phase::Playing && nowMs - m_blinkMs >= kSecondMs) {
            --m_timeLeft;
            m_blinkMs = nowMs;
            if (m_timeLeft < 0) {
                m_phase = Phase::Countdown;
            }
        }
    }

    void LuckyGame::addObject(int kind, const char* picture, int frames) {
        const core::Size size = m_hooks.pictureSize(picture);
        for (FallingObject& object : m_objects) {
            if (!object.inUse) {
                object.x = m_throwerX;
                object.y = kThrowerY - kSpawnRise;
                object.previousY = object.y;
                object.kind = kind;
                object.lastFrame = frames - 1;
                object.frameHeight = size.height / frames;
                object.frameWidth = size.width;
                object.halfWidth = object.frameWidth / 2;
                object.anchorY = object.frameHeight;
                object.inUse = true;
                object.fallSpeed = kFallSpeed;
                break;
            }
        }
    }

    std::string LuckyGame::overText() const {
        if (m_hearts == 0) {
            return "Game Over";
        }
        return m_score >= kWinScore ? "You Win" : "You Lose";
    }

    void LuckyGame::drawStep(std::uint64_t nowMs) {
        m_playerFrame = m_playerAnim.next(nowMs, m_hooks.playerFrames(m_playerSlot), isLyingGroup(m_playerSlot));
        m_throwerFrame = m_throwerAnim.next(nowMs, m_hooks.throwerFrames(m_throwerSlot));
        if (m_phase == Phase::Playing) {
            for (std::size_t i = 0; i < m_objects.size(); ++i) {
                FallingObject& object = m_objects[i];
                if (object.inUse) {
                    m_shownFrames[i] = object.curFrame;
                    object.curFrame = object.curFrame < object.lastFrame ? object.curFrame + 1 : 0;
                }
            }
        }
        if (m_phase == Phase::Over && nowMs - m_overMs >= kOverMs) {
            finish(m_hearts != 0 && m_score >= kWinScore ? 1 : 0); // result 1: won, 0: lost
        }
    }

    void LuckyGame::frame(std::uint64_t nowMs, bool instructionsOpen, int mouseX) {
        if (!instructionsOpen) {
            m_formShown = true;
            countdownStep(nowMs);
            if (m_phase == Phase::Playing) {
                moveToMouse(mouseX);
                bounceThrower();
                fallObjects();
                checkCatches();
                if (m_hearts == 0 || m_timeLeft == 0) {
                    m_phase = Phase::Over;
                    m_overMs = nowMs;
                }
                const int distance = m_lastDropX != 0 ? std::abs(m_throwerX - m_lastDropX) : 0;
                const int roll = random(kDropChances);
                if (roll <= 1) { // rolls 0 and 1 of 15 drop a coin
                    if (m_lastDropX == 0 || m_hooks.pictureSize(kCoinPicture).width < distance) {
                        addObject(kCoin, kCoinPicture, 1); // the coin picture has one frame
                        m_lastDropX = m_throwerX;
                    }
                }
                if (roll == 2) { // roll 2 of 15 drops a bomb
                    if (m_lastDropX == 0 || m_hooks.pictureSize(kBombPicture).width < distance) {
                        addObject(kBomb, kBombPicture, kBombFrames);
                        m_lastDropX = m_throwerX;
                    }
                }
                tickClock(nowMs);
            }
        }
        drawStep(nowMs);
    }

    void LuckyGame::update(std::uint64_t nowMs, bool instructionsOpen, int mouseX) {
        constexpr int kMostCatchUp = 10; // most frames run in one update, to avoid a spiral after a stall
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_previousPlayerX = m_playerX;
            m_previousThrowerX = m_throwerX;
            for (FallingObject& object : m_objects) {
                object.previousY = object.y;
            }
            frame(m_lastFrameMs, instructionsOpen, mouseX);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
