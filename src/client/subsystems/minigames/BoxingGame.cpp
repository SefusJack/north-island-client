#include "client/subsystems/minigames/BoxingGame.h"

#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kPointsCost = 75; // IM points paid for one game
        constexpr int kTokenCost = 15; // tokens paid for one game
        constexpr int kMessageMs = 1300; // how long a short message shows, ms
        constexpr int kRewardMessageMs = 3500; // how long the reward message shows, ms
        constexpr int kGaugeStep = 30; // power gauge step per frame, px (also the score per gauge level)
        constexpr int kMostHoldFrames = 7; // frames the punch pose is held
        constexpr int kBoxerAdvanceTicks = 4; // ticks before the boxer advances to its punch frame
        constexpr int kRewardStop = 370; // y the prize figure drops to, px
        constexpr int kRewardStepY = 10; // px the prize figure moves per frame
        constexpr int kRewardLastStep = 50; // reward step at which the reward sequence ends
        const char* const kMusic = "BGM0019"; // background music (sound/BGM0019.wav)
        const char* const kFull = "Item bar is full\rNot enough space for the rewards";

        /// @brief The monster's frame by the punches made.
        /// @details Original: DrawBoxingPunchFrame.
        /// @param punches Number of punches made so far.
        /// @return Monster frame.
        int monsterFrameFor(int punches) {
            return punches == 1 ? 1 : (punches == 2 || punches == 3) ? 2 : 0;
        } // frame 1 after one punch, 2 after two or three, else 0

    }

    BoxingGame::BoxingGame(
        GameHooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void BoxingGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_formShown = false;
        m_paid = false;
        m_payKind = 1;
        m_gaugeRising = true;
        m_powerWidth = 0;
        m_powerLevel = 0;
        m_punchRequested = false;
        m_punchBlocked = false;
        m_rewardShown = false;
        m_overPunch = false;
        m_punchCount = 0;
        m_punchScore = 0;
        m_holdFrames = 0;
        m_boxerFrame = 0;
        m_boxerTick = 60; // above the advance ticks, so the boxer's first advance comes at once
        m_rewardStep = 0;
        m_rewardY = 30; // start height of the prize figure
        m_previousPowerWidth = 0;
        m_previousRewardY = 30; // start height of the prize figure
        m_answer = 0;
        m_quitting = false;
        m_rewardText.clear();
        m_pose = {};
        m_scores = {};
        m_hooks.stopMusic();
        m_hooks.send(protocol::imPointsRequest(0));
    }

    void BoxingGame::startGame() {
        m_quitting = false;
        m_rewardText.clear();
        m_overPunch = false;
        m_gaugeRising = true;
        m_punchRequested = false;
        m_punchCount = 0;
        m_powerLevel = 0;
        m_punchBlocked = false;
        m_punchScore = 0;
        m_holdFrames = 0;
        m_powerWidth = 0;
        m_rewardY = 30; // start height of the prize figure
        m_previousPowerWidth = 0;
        m_previousRewardY = 30; // start height of the prize figure
        m_rewardStep = 0;
        m_scores = {};
    }

    void BoxingGame::moved(bool overPunchButton) {
        if (!m_punchRequested && !m_rewardShown) {
            m_overPunch = overPunchButton;
        }
    }

    void BoxingGame::pressed(bool instructionsOpen) {
        if (instructionsOpen) {
            return;
        }
        if (!m_paid) {
            askToPay();
            return;
        }
        if (m_overPunch && !m_punchBlocked && !m_punchRequested && !m_rewardShown) {
            m_punchRequested = true;
        }
    }

    void BoxingGame::askToPay() {
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        const int tokens = m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken);
        if (tokens != 0) {
            m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int choice) { chooseWay(choice); });
        } else if (hasRewardSpace()) {
            m_hooks.ask(
                points + "\rThe points for playing Golden Glove " + std::to_string(kPointsCost) + " Points!\r Sure to continue?",
                false,
                [this] { playWith(1); },
                [] {}
            );
        } else {
            m_hooks.message(kFull, kMessageMs);
        }
    }

    void BoxingGame::chooseWay(int choice) {
        if (!hasRewardSpace()) {
            m_hooks.message(kFull, kMessageMs);
            return;
        }
        // list choices: 0 points, 1 Carnie Token, 2 Fun Token; pay kinds: 1 points, 2 Carnie Token, 3 Fun Token
        if (choice == 0) {
            const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
            m_hooks.ask(
                points + "\rThe points for playing Golden Glove " + std::to_string(kPointsCost) + " Points!\r Sure to continue?",
                true,
                [this] { playWith(1); },
                [] {}
            );
        } else if (choice == 1 || choice == 2) {
            const int payKind = choice + 1;
            const std::string rest = "Rest of the tokens:" + std::to_string(m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken));
            m_hooks.ask(
                rest + "\rThe token for playing Golden Glove " + std::to_string(kTokenCost) + "\rContinue\xA1H",
                true,
                [this, payKind] { playWith(payKind); },
                [] {}
            );
        }
    }

    void BoxingGame::playWith(int payKind) {
        // pay kinds: 1 points, 2 Carnie Token, 3 Fun Token
        if (payKind == 1) {
            if (m_hooks.imPoints() < kPointsCost) {
                m_hooks.message("Please supply more points", kMessageMs);
                return;
            }
        } else if (m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken) < kTokenCost) {
            m_hooks.message("Not enough carnie tokens", kMessageMs);
            return;
        }
        if (!hasRewardSpace()) {
            m_hooks.message(kFull, kMessageMs);
            return;
        }
        startGame();
        m_payKind = payKind;
        m_paid = true;
        m_hooks.playMusic(kMusic);
    }

    void BoxingGame::offerReplay() {
        const int payKind = m_payKind;
        std::string text;
        if (payKind == 2 || payKind == 3) {
            text = "Rest of the tokens:" + std::to_string(m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken)) + "\rPlay again? Need more tokens " +
                   std::to_string(kTokenCost);
        } else {
            text = "IM Points :" + std::to_string(m_hooks.imPoints()) + "\rPlay again? Need more points " + std::to_string(kPointsCost);
        }
        m_hooks.ask(text, false, [this, payKind] { playWith(payKind); }, [] {});
    }

    void BoxingGame::exit() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void BoxingGame::sendQuit() {
        m_answer = 2; // answer 2: quit
        m_quitting = true;
        // packet 71/9: the play request (pay kind, answer)
        m_hooks.send({71, 9, static_cast<std::uint8_t>(m_payKind), static_cast<std::uint8_t>(m_answer)});
    }

    void BoxingGame::quitClicked() {
        const bool tokens = m_payKind == 2;
        const std::string exitText = tokens ? "Going to exit\xA1H\r\xA1mDon't need lose token now\xA1n" : "Want to exit the game?\r[Won't lose points now]";
        const std::string quitText = tokens ? "The result would depend on your recent state\rWant to exit\xA1H\r\xA1mNeed to lose token\xA1n"
                                            : "The result would depend on your recent state\rContinue\xA1H\r\xA1mThe recharge points would be decreased\xA1n";
        if (!m_paid) {
            exit();
        } else if (m_punchCount == 0) {
            m_hooks.ask(exitText, false, [this] { exit(); }, [] {});
        } else if (m_punchCount <= 2) { // quitting early is possible up to two punches made
            m_hooks.ask(quitText, false, [this] { sendQuit(); }, [] {});
        }
    }

    void BoxingGame::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the 2-byte packet header (71, 9)
        const int code = reader.u8();
        if (code == 2) { // code 2: the play request is refused, not enough points
            m_paid = false;
            m_hooks.requestMessage("Please supply more points", kMessageMs);
            return;
        }
        if (code != 3 && code != 4) { // codes 3 and 4: the reward
            return;
        }
        const int itemId = reader.i32();
        const int quantity = reader.u8();
        if (reader.failed()) {
            return;
        }
        const std::string name = m_hooks.itemName(itemId);
        m_rewardText = m_scores.left == 0 ? "Get rewards\xA1G" + std::to_string(quantity) + " Quantity " + name
                                          : "Got an honorable prize " + std::to_string(quantity) + " Quantity " + name;
        if (code == 4 && m_quitting) { // code 4: the reward after quitting
            m_hooks.requestMessage("The result is\xA1G\rGot an honorable prize " + std::to_string(quantity) + " Quantity [ " + name + " ]", kRewardMessageMs);
            m_hooks.playSound("sound/wav0152.wav");
            exit();
        }
    }

    void BoxingGame::updateGauge() {
        if (m_rewardShown) {
            return;
        }
        if (!m_punchRequested) {
            if (m_gaugeRising) {
                ++m_powerLevel;
                if (m_powerWidth + kGaugeStep <= kGaugeWidth) {
                    m_powerWidth += kGaugeStep;
                } else {
                    m_powerWidth = kGaugeWidth;
                    m_gaugeRising = false;
                }
            } else {
                --m_powerLevel;
                if (m_powerWidth - kGaugeStep >= 0) {
                    m_powerWidth -= kGaugeStep;
                } else {
                    m_powerWidth = 0;
                    m_gaugeRising = true;
                    m_powerLevel = 0;
                }
            }
        } else if (!m_punchBlocked) {
            m_punchBlocked = true;
            m_punchScore = random(10) + m_powerLevel * kGaugeStep; // 0..9 at random plus 30 per gauge level
        }
    }

    void BoxingGame::drawPunch() {
        if (m_punchRequested) {
            Pose pose;
            pose.lit = m_overPunch;
            if (m_boxerFrame == 0) {
                pose.monsterFrame = monsterFrameFor(m_punchCount);
                if (m_boxerTick >= kBoxerAdvanceTicks) {
                    m_boxerTick = 0;
                    ++m_boxerFrame;
                } else {
                    ++m_boxerTick;
                }
            }
            if (m_boxerFrame < 1) {
                m_pose = pose;
                return;
            }
            pose.monsterFrame = 3; // monster frame 3: its hit frame
            pose.boxerFrame = 1;
            pose.shake = random(2) == 1 ? random(3) : -random(3); // shake of 0..2 pixels either way
            m_pose = pose;
            if (m_holdFrames <= kMostHoldFrames) {
                ++m_holdFrames;
                return;
            }
            // the sound by the punch score: up to 100 weak, up to 299 middle, above strong
            m_hooks.playSound(m_punchScore <= 100 ? "sound/SEB0024.wav" : m_punchScore <= 299 ? "sound/SEB0026.wav" : "sound/SEB0052.wav");
            m_boxerFrame = 0;
            m_punchRequested = false;
            if (m_punchCount < kPunches - 1) {
                return;
            }
            m_rewardShown = true;
            m_overPunch = false;
            return;
        }
        if (m_punchBlocked) {
            ++m_punchCount;
        }
        m_pose = {monsterFrameFor(m_punchCount), 0, 0, m_overPunch};
        scoreBoard();
        m_punchBlocked = false;
        m_holdFrames = 0;
    }

    void BoxingGame::scoreBoard() {
        if (m_punchCount >= 1 && m_punchCount <= kPunches) {
            m_scores.punch[m_punchCount - 1] = m_punchScore;
            m_scores.left = kTotalScore;
            for (int i = 0; i < m_punchCount; ++i) {
                m_scores.left -= m_scores.punch[i];
            }
        }
    }

    void BoxingGame::drawReward() {
        if (!m_rewardShown) {
            return;
        }
        if (m_rewardStep == 0 && m_punchCount == kPunches) {
            m_answer = m_scores.left != 0 ? 0 : 1; // answer 0: lost, 1: knock-out
            // packet 71/9: the play request (pay kind, answer)
            m_hooks.send({71, 9, static_cast<std::uint8_t>(m_payKind), static_cast<std::uint8_t>(m_answer)});
            ++m_rewardStep;
        }
        // reward steps: 1 down, 2 up (by 40 pixels), 3 down again, 4 message and sound, 5 on wait until the last step
        if (m_rewardStep == 1) {
            if (m_rewardY + kRewardStepY <= kRewardStop) {
                m_rewardY += kRewardStepY;
            } else {
                ++m_rewardStep;
            }
        }
        if (m_rewardStep == 2) {
            if (m_rewardY - kRewardStepY >= kRewardStop - 40) {
                m_rewardY -= kRewardStepY;
            } else {
                ++m_rewardStep;
            }
        }
        if (m_rewardStep == 3) {
            if (kRewardStepY * 2 + m_rewardY <= kRewardStop) {
                m_rewardY += kRewardStepY;
            } else {
                ++m_rewardStep;
            }
        }
        if (m_rewardStep == 4) {
            m_hooks.requestMessage(m_rewardText, kRewardMessageMs);
            m_hooks.playSound("sound/wav0152.wav");
            ++m_rewardStep;
        }
        if (m_rewardStep >= 5 && m_rewardStep <= kRewardLastStep - 1) {
            ++m_rewardStep;
        }
        if (m_rewardStep == kRewardLastStep) {
            m_rewardText.clear();
            m_rewardShown = false;
            m_paid = false;
            offerReplay();
        }
    }

    void BoxingGame::frame(bool instructionsOpen) {
        m_formShown = !instructionsOpen;
        if (m_paid) {
            updateGauge();
        }
        drawPunch();
        if (m_scores.left < 0) {
            m_scores.left = 0;
        }
        drawReward();
    }

    void BoxingGame::update(std::uint64_t nowMs, bool instructionsOpen) {
        constexpr int kMostCatchUp = 10; // most frames run at once when the game fell behind
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_previousPowerWidth = m_powerWidth;
            m_previousRewardY = m_rewardY;
            frame(instructionsOpen);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
