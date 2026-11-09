#include "client/subsystems/minigames/JigsawGame.h"

#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kMessageMs = 1300; // short message display time, ms
        constexpr int kRewardMessageMs = 3500; // reward message display time, ms
        constexpr std::uint64_t kReelSoundMs = 800; // interval of the reel spinning sound, ms
        constexpr int kStripLastFrame = 15; // last frame index of a reel strip (16 frames, 8 symbols two frames apart)
        constexpr int kBingoLastFrame = 1; // last frame index of the bingo animation (2 frames)
        constexpr int kFramesPerStop = 4; // strip frame steps a reel spins before it may stop
        constexpr int kFirstBingoStage = 10; // reel stage where the bingo or flash animation starts
        constexpr int kLastSpinStage = 40; // last reel stage of the bingo or flash animation
        constexpr int kRewardStage = 41; // reel stage that announces the reward
        constexpr int kFlashFrames = 5; // frames between flash toggles
        constexpr int kFlashRowsShifted = 210; // flash rows offset while the flash is off, px
        const char* const kMusic = "BGM0019"; // background music name
        const char* const kFull = "Item bar is full\rNot enough space for the rewards";

        // table: per strip (row 1..3), the strip frame shown at server stop 0..8
        constexpr int kReelSymbols[4][9] =
            {{0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 4, 8, 10, 14, 2, 12, 0, 6}, {0, 0, 14, 12, 8, 10, 6, 4, 2}, {0, 10, 8, 6, 4, 0, 2, 14, 12}};
        constexpr int kStartFrames[3] = {4, 0, 10}; // strip frame each of the 3 strips starts on
        constexpr int kDelays[3] = {3, 4, 5}; // frames each of the 3 strips holds a picture (spin speed)
        // prizes of kind 14, index 0 = no prize, 1..10 = prize numbers
        constexpr MachineTables::Prizes kThirdPrizes{
            30063, // Lottery Ticket (index 0, no prize)
            62009, // 26% Bomb Diamond
            34152, // Tear Plier
            21159, // Defense Armor
            24125, // Gold Boots
            23132, // Gold Gloves
            34020, // Magic Set Box
            34015, // Glazed Stone Bag
            34016, // Encyclopedia
            32162, // Pizza
            33019 // Recovery Potion
        };

    }

    JigsawGame::JigsawGame(
        int kind,
        GameHooks hooks
    )
        : m_kind(kind),
          m_hooks(std::move(hooks)) {}

    const MachineTables::Prizes& JigsawGame::prizes() const {
        // machine kinds 11 and 12 (Jigsaw machines; kind 14 uses kThirdPrizes) and the slot-table numbers 10 and 8 of MachineTables
        return m_kind == 11 ? MachineTables::slotPrizes(10) : m_kind == 12 ? MachineTables::slotPrizes(8) : kThirdPrizes;
    }

    // points per play: 25 (kind 11), 100 (kind 12), 10 (kind 14)
    int JigsawGame::cost() const { return m_kind == 11 ? 25 : m_kind == 12 ? 100 : 10; }

    // tokens per play: 20 for kind 12, else 2
    int JigsawGame::tokenCost() const { return m_kind == 12 ? 20 : 2; }

    void JigsawGame::resetStrips() {
        for (std::size_t strip = 0; strip < kStrips; ++strip) {
            m_strips[strip].reset();
            m_strips[strip].frame = kStartFrames[strip];
            m_strips[strip].delay = kDelays[strip];
            m_strips[strip].count = kStripLastFrame;
            m_shown[strip] = m_strips[strip].frame;
        }
    }

    void JigsawGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_formShown = false;
        m_betPlaced = false;
        m_reelsStopped = false;
        m_started = false;
        m_resultReceived = false;
        m_rewardPending = false;
        m_prizeWon = false;
        m_bingoFlash = false;
        m_confirmed = false;
        m_reelSoundOn = true;
        m_payKind = 1; // pay kind 1: points
        m_spinRounds = 0;
        m_reelStage = 0;
        m_frameCounter = 0;
        m_reelSoundMs = 0;
        m_symbols = {};
        m_bingo = {};
        m_bingo.count = kBingoLastFrame;
        m_bingoShown = -1;
        m_flashOn = false;
        m_flashTick = 0;
        m_flashShown = false;
        m_rewardText.clear();
        resetStrips();
        m_hooks.stopMusic();
        m_hooks.send(protocol::imPointsRequest(0));
    }

    void JigsawGame::begin() { m_confirmed = false; }

    void JigsawGame::startGame() {
        m_rewardText.clear();
        m_started = false;
        m_resultReceived = false;
        m_rewardPending = false;
        m_prizeWon = false;
        m_betPlaced = false;
        m_reelSoundOn = true;
        m_bingoFlash = false;
        m_spinRounds = 0;
        m_reelStage = 0;
        m_frameCounter = 0;
        m_reelSoundMs = 0;
        m_symbols = {};
        m_bingoShown = -1;
    }

    void JigsawGame::exitGame() {
        if (m_rewardPending) {
            m_hooks.requestMessage(m_rewardText, kRewardMessageMs);
            m_hooks.playSound("sound/wav0152.wav");
            m_rewardPending = false;
        }
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    std::string JigsawGame::pointsText() const { return "IM Points :" + std::to_string(m_hooks.imPoints()); }

    std::string JigsawGame::questionPoints() const {
        return pointsText() + "\rThe points for playing Jigsaw " + std::to_string(cost()) + " Points!\r Sure to continue?";
    }

    std::string JigsawGame::questionTokens(int payKind) const {
        return "Rest of the tokens:" + std::to_string(m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken)) + "\rThe token for playing Jigsaw " +
               std::to_string(tokenCost()) + "\rContinue\xA1H";
    }

    void JigsawGame::chooseWay() {
        if (m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken) != 0) {
            m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int choice) { askToPay(choice); });
        } else {
            m_hooks.ask(questionPoints(), false, [this] { playWithPoints(); }, [] {});
        }
    }

    void JigsawGame::askToPay(int choice) {
        if (!hasRewardSpace()) {
            m_hooks.message(kFull, kMessageMs);
            return;
        }
        if (choice == 0) {
            m_hooks.ask(questionPoints(), true, [this] { playWithPoints(); }, [] {});
        } else if (choice == 1 || choice == 2) {
            const int payKind = choice + 1;
            m_hooks.ask(questionTokens(payKind), true, [this, payKind] { playWithTokens(payKind); }, [] {});
        }
    }

    void JigsawGame::playWithPoints() {
        m_confirmed = false;
        if (m_hooks.imPoints() < cost()) {
            m_hooks.message("Please supply more points", kMessageMs);
            return;
        }
        if (hasRewardSpace()) {
            m_payKind = 1;
            startGame();
            m_betPlaced = true;
            m_reelsStopped = false;
            m_hooks.playMusic(kMusic);
        } else {
            m_hooks.message(kFull, kMessageMs);
        }
        m_confirmed = true;
    }

    void JigsawGame::playWithTokens(int payKind) {
        m_confirmed = false;
        if (m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken) < tokenCost()) {
            m_hooks.message("Not enough carnie tokens", kMessageMs);
            return;
        }
        if (hasRewardSpace()) {
            m_payKind = payKind;
            startGame();
            m_betPlaced = true;
            m_reelsStopped = false;
            m_hooks.playMusic(kMusic);
        } else {
            m_hooks.message(kFull, kMessageMs);
        }
        m_confirmed = true;
    }

    void JigsawGame::askPlayAgain() {
        if (m_confirmed) {
            if (m_kind == 11 || m_payKind == 1) {
                playWithPoints();
            } else {
                playWithTokens(m_payKind);
            }
            return;
        }
        if (m_kind == 11 || m_payKind == 1) {
            m_hooks.ask(pointsText() + "\rPlay again? Need more points " + std::to_string(cost()), false, [this] { playWithPoints(); }, [] {});
        } else {
            const int payKind = m_payKind;
            m_hooks.ask(
                "Rest of the tokens:" + std::to_string(m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken)) + "\rPlay again? Need more tokens " +
                    std::to_string(tokenCost()),
                false,
                [this, payKind] { playWithTokens(payKind); },
                [] {}
            );
        }
    }

    void JigsawGame::sendPlay() {
        if (m_kind == 11) {
            m_hooks.send({71, 11}); // packet 71, kind 11: play request
        } else {
            m_hooks.send({71, static_cast<std::uint8_t>(m_kind), static_cast<std::uint8_t>(m_payKind)}); // packet 71, kind, pay kind: play request
        }
    }

    void JigsawGame::startClicked() {
        if (!m_betPlaced || m_reelsStopped) {
            if (m_kind == 11) {
                if (!hasRewardSpace()) {
                    m_hooks.message("Not enough item bar\rNot enough space for rewards", kMessageMs);
                } else if (!m_confirmed) {
                    m_hooks.ask(questionPoints(), false, [this] { playWithPoints(); }, [] {});
                } else {
                    playWithPoints();
                }
                return;
            }
            if (!m_confirmed) {
                chooseWay();
                return;
            }
        }
        if (m_betPlaced && !m_started) {
            m_started = true;
            sendPlay();
        }
    }

    void JigsawGame::quitClicked() {
        if (m_betPlaced || m_reelsStopped) {
            if (!m_started) {
                m_hooks.ask("Want to exit the game?\r[Won't lose points now]", false, [this] { exitGame(); }, [] {});
            }
            if (m_reelsStopped) {
                exitGame();
            }
        } else {
            exitGame();
        }
    }

    void JigsawGame::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int code = reader.u8(); // sub code: 2 refused, 3 spin result, 4 bingo variant, 5 no room
        if (code == 2) { // 71/kind sub 2: play refused, not enough points
            m_betPlaced = false;
            m_hooks.requestMessage("Please supply more points", kMessageMs);
            return;
        }
        if (code == 5) { // 71/kind sub 5: no room for the prize
            m_hooks.message("Not enough item bar\rNot enough space for rewards", kMessageMs);
            return;
        }
        if (code != 3 && code != 4) { // sub 3: spin result, sub 4: bingo variant
            return;
        }
        const int prize = reader.u8();
        for (int strip = 1; strip <= kStrips; ++strip) {
            const int stop = reader.u8();
            m_symbols[static_cast<std::size_t>(strip)] = stop >= 0 && stop <= 8 ? kReelSymbols[strip][stop] : 0; // highest stop number is 8
        }
        const int resultKind = code == 4 ? reader.u8() : 0; // sub 4 carries a result kind byte
        const int quantity = reader.u8();
        if (reader.failed() || prize > MachineTables::kPrizes) {
            return;
        }
        const std::string name = m_hooks.itemName(prizes()[static_cast<std::size_t>(prize)]);
        // kind 12 with sub 3 names only the prize, others add the quantity
        m_rewardText =
            m_kind == 12 && code == 3 ? "Get rewards\xA1G[ " + name + " ]" : "Get rewards\xA1G" + std::to_string(quantity) + " Quantity [ " + name + " ]";
        m_resultReceived = true;
        m_rewardPending = true;
        if (code == 3 && prize != 0) {
            m_prizeWon = true;
        }
        if (code == 4 && resultKind == 1) { // result kind 1: bingo flash
            m_bingoFlash = true;
        }
    }

    void JigsawGame::spinStrips(std::uint64_t nowMs) {
        FramePicture& up = m_strips[0];
        FramePicture& mid = m_strips[1];
        FramePicture& bottom = m_strips[2];
        int& shownUp = m_shown[0];
        int& shownMid = m_shown[1];
        int& shownBottom = m_shown[2];
        m_bingoShown = -1;
        m_flashShown = false;
        if (!m_resultReceived) {
            for (std::size_t strip = 0; strip < kStrips; ++strip) {
                m_shown[strip] = m_strips[strip].frame;
            }
            return;
        }
        if (m_reelSoundOn && nowMs - m_reelSoundMs >= kReelSoundMs) {
            m_hooks.playSound("sound/9912.wav");
            m_reelSoundMs = nowMs;
        }
        const auto upAdvance = [&] {
            const int before = up.frame;
            shownUp = up.advance();
            if (up.frame != before) {
                ++m_frameCounter;
            }
            if (up.isLast() && up.tick == up.delay) {
                up.reset();
            }
        };
        const auto midStepBack = [&] {
            if (mid.isFirst() && mid.tick == mid.delay) {
                mid.frame = mid.count;
                mid.tick = 0;
            }
            shownMid = mid.stepBack();
        };
        const auto bottomAdvance = [&] {
            shownBottom = bottom.advance();
            if (bottom.isLast() && bottom.tick == bottom.delay) {
                bottom.reset();
            }
        };
        const auto shown = [&](bool upOn, bool midOn, bool bottomOn) {
            if (upOn) {
                shownUp = m_symbols[1];
            }
            if (midOn) {
                shownMid = m_symbols[2];
            }
            if (bottomOn) {
                shownBottom = m_symbols[3];
            }
        };
        if (m_spinRounds == 0) {
            upAdvance();
            if (m_frameCounter >= kFramesPerStop) {
                m_frameCounter = 0;
                ++m_spinRounds;
            }
            midStepBack();
            bottomAdvance();
            return;
        }
        switch (m_reelStage) {
            case 0: // stage 0: the first strip is stopping
                if (up.frame == m_symbols[1] && up.tick == up.delay && m_frameCounter >= kFramesPerStop) {
                    m_frameCounter = 0;
                    ++m_reelStage;
                    m_hooks.playSound("sound/9911.wav");
                }
                upAdvance();
                if (m_reelStage == 0 && mid.isFirst() && mid.tick == mid.delay) {
                    mid.frame = mid.count;
                    mid.tick = 0;
                }
                shownMid = mid.stepBack();
                bottomAdvance();
                break;
            case 1: { // stage 1: the middle strip is stopping
                shown(true, false, false);
                if (mid.frame == m_symbols[2] && mid.tick == mid.delay && m_frameCounter >= kFramesPerStop) {
                    m_frameCounter = 0;
                    ++m_reelStage;
                    m_hooks.playSound("sound/9911.wav");
                }
                if (mid.isFirst() && mid.tick == mid.delay) {
                    mid.frame = mid.count;
                    mid.tick = 0;
                }
                const int before = mid.frame;
                if (m_reelStage == 1) {
                    shownMid = mid.stepBack();
                } else {
                    shown(false, true, false);
                }
                if (mid.frame != before) {
                    ++m_frameCounter;
                }
                bottomAdvance();
                break;
            }
            case 2: { // stage 2: the bottom strip is stopping
                shown(true, true, false);
                if (bottom.frame == m_symbols[3] && bottom.tick == bottom.delay && m_frameCounter >= kFramesPerStop) {
                    m_frameCounter = 0;
                    ++m_reelStage;
                    m_hooks.stopSounds();
                    m_hooks.playSound("sound/9911.wav");
                    m_reelSoundOn = false;
                }
                const int before = bottom.frame;
                if (m_reelStage == 2) {
                    shownBottom = bottom.advance();
                } else {
                    shown(false, false, true);
                }
                if (bottom.frame != before) {
                    ++m_frameCounter;
                }
                if (bottom.isLast() && bottom.tick == bottom.delay) {
                    bottom.reset();
                }
                break;
            }
            case kRewardStage: // final stage: announce the reward and ask to play again
                shown(true, true, true);
                m_hooks.requestMessage(m_rewardText, kRewardMessageMs);
                m_hooks.playSound("sound/wav0152.wav");
                m_reelsStopped = true;
                m_rewardPending = false;
                for (std::size_t strip = 0; strip < kStrips; ++strip) {
                    m_strips[strip].frame = m_symbols[strip + 1];
                    m_strips[strip].tick = 0;
                }
                ++m_reelStage;
                askPlayAgain();
                break;
            default: // stages 3 to 40: all strips stopped, bingo or flash plays
                shown(true, true, true);
                if (m_reelStage >= 3 && m_reelStage <= kLastSpinStage) {
                    ++m_reelStage;
                    if (m_prizeWon) {
                        if (m_reelStage >= kFirstBingoStage) {
                            m_bingoShown = m_bingo.isLast() ? m_bingo.stepBack() : m_bingo.advance();
                        }
                    } else if (m_bingoFlash) {
                        if (m_reelStage >= kFirstBingoStage) {
                            ++m_flashTick;
                            if (m_flashTick >= kFlashFrames) {
                                m_flashOn = !m_flashOn;
                                m_flashTick = 0;
                            }
                            m_flashRows = m_flashOn ? 0 : kFlashRowsShifted;
                            m_flashShown = true;
                        }
                    } else {
                        m_reelStage = kRewardStage;
                    }
                }
                break;
        }
    }

    void JigsawGame::frame(std::uint64_t nowMs) { spinStrips(nowMs); }

    void JigsawGame::update(std::uint64_t nowMs, bool instructionsOpen) {
        constexpr int kMostCatchUp = 10; // most frames run in one update, to avoid a spiral after a stall
        m_formShown = !instructionsOpen;
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            frame(nowMs);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
