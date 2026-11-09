#include "client/subsystems/minigames/SlotMachine.h"

#include "client/subsystems/minigames/MachineTables.h"
#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

#include <string>

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kLuckyStar = 34360; // item 34360: Lucky Star
        constexpr int kMessageMs = 1000; // on-screen message duration, ms
        constexpr int kShortMessageMs = 800; // short on-screen message duration, ms
        constexpr std::uint64_t kFlashMs = 2400; // prize flash duration, ms
        constexpr std::uint64_t kFlashStepMs = 150; // time per flash step, ms
        constexpr std::uint64_t kBannerPixelMs = 10; // time per banner scroll pixel, ms
        const char* const kFull = "Sorry, your inventory is full.";
        const char* const kMusic = "BGM0019";
        const char* const kSpinSound = "sound/wav9915.wav";
        const char* const kPullSound = "sound/wav9916.wav";
        const char* const kPrizeSound = "sound/wav0152.wav";
        const char* const kFreePlay = "You get a free spin! Good luck!";

    }

    SlotMachine::SlotMachine(
        int kind,
        GameHooks hooks
    )
        : m_kind(kind),
          m_hooks(std::move(hooks)) {}

    bool SlotMachine::freePlay() const { return m_hooks.freePlay && m_hooks.freePlay(); }

    // points per play: kind 8 costs 10, kind 10 costs 5, kind 19 costs 20
    int SlotMachine::pointCost() const { return m_kind == 8 ? 10 : (m_kind == 10 ? 5 : 20); }

    // tokens per play: kind 8 costs 2, kind 10 none, kind 19 costs 4
    int SlotMachine::tokenCost() const { return m_kind == 8 ? 2 : (m_kind == 10 ? 0 : 4); }

    void SlotMachine::open(std::uint64_t nowMs) {
        m_musicOn = false;
        m_active = false;
        m_exitShown = true;
        m_askPayment = true;
        m_paid = false;
        m_startConfirmed = false;
        m_payKind = 1;
        m_spinning = false;
        m_spinSoundPending = false;
        m_resultShown = false;
        m_wonPrize = false;
        m_prizeKnown = false;
        m_prizeFlash = false;
        m_spinStartMs = nowMs;
        m_reelMs = nowMs;
        m_bannerStartMs = 0;
        for (int reel = 1; reel <= kReels; ++reel) {
            m_reelPos[static_cast<std::size_t>(reel)] = 1;
            m_reelTarget[static_cast<std::size_t>(reel)] = 1;
            m_reelStopped[static_cast<std::size_t>(reel)] = false;
        }
        m_prizeCount = 0;
        m_luckyStarShown = false;
        m_luckyStarOn = false;
        m_luckyStarPlace = 0;
        m_hooks.stopMusic();
        m_hooks.send(protocol::imPointsRequest(0));
    }

    void SlotMachine::begin() {
        m_startConfirmed = false;
        m_active = true;
        m_hooks.send(protocol::imPointsRequest(0));
        findLuckyStar();
    }

    void SlotMachine::findLuckyStar() {
        m_luckyStarShown = false;
        m_luckyStarOn = false;
        m_luckyStarPlace = 0;
        if (const int place = m_hooks.freeItemPlace(kLuckyStar)) {
            m_luckyStarPlace = place;
            m_luckyStarShown = true;
        }
    }

    void SlotMachine::luckyStarClicked(bool explicitClick) {
        if (m_spinning) {
            m_hooks.message("Can't be used during the game", kMessageMs);
            return;
        }
        if (!m_luckyStarShown) {
            return;
        }
        if (!m_luckyStarOn) {
            if (m_hooks.itemInPlace(m_luckyStarPlace) != kLuckyStar) {
                m_luckyStarPlace = m_hooks.freeItemPlace(kLuckyStar);
                if (m_luckyStarPlace == 0) {
                    if (explicitClick) {
                        m_hooks.message(" Has no " + m_hooks.itemName(kLuckyStar) + ",Use failed", kMessageMs);
                    }
                    return;
                }
            }
            m_luckyStarOn = true;
        } else {
            m_luckyStarOn = false;
        }
    }

    void SlotMachine::leave() { exitGame(); }

    void SlotMachine::exitClicked() {
        m_active = false;
        exitGame();
    }

    void SlotMachine::startMusicWhenActive() {
        if (m_active && !m_musicOn) {
            m_hooks.playMusic(kMusic);
            m_musicOn = true;
        }
    }

    void SlotMachine::showObtained(bool withCount) {
        const std::string name = m_hooks.itemName(m_prizeItem);
        const std::string quantity = withCount ? "Obtain " + std::to_string(m_prizeCount) + " Quantity " : "Obtain ";
        m_hooks.message(quantity + " [" + name + "]\xA1I", kMessageMs);
        m_hooks.playSound(kPrizeSound);
    }

    void SlotMachine::exitGame() {
        if (!m_resultShown && m_prizeKnown) {
            showObtained(false);
            m_resultShown = true;
            m_wonPrize = false;
        }
        m_musicOn = false;
        m_hooks.stopSounds();
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    std::string SlotMachine::pointsQuestion() const {
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        switch (m_kind) {
            case 19: // machine kind 19: its own wording
                return points + "\rPlaying this game needs points " + std::to_string(pointCost()) + " points\rSure to play\xA1H";
            case 10: // machine kind 10: asks "Continue"
                return points + "\rThe points of playing it once " + std::to_string(pointCost()) + " points\rContinue\xA1H";
            default: // machine kind 8: asks "Sure to play"
                return points + "\rThe points of playing it once " + std::to_string(pointCost()) + " points\rSure to play\xA1H";
        }
    }

    std::string SlotMachine::tokensQuestion(int tokens) const {
        const std::string rest = "Rest of the tokens:" + std::to_string(tokens);
        if (m_kind == 19) { // machine kind 19: the stone pull machine
            return rest + "\rPlaying stone pull machine needs token " + std::to_string(tokenCost()) + "\rContinue\xA1H";
        }
        return rest + "\rThe token for playing it once " + std::to_string(tokenCost()) + "\rContinue\xA1H";
    }

    void SlotMachine::playClicked(std::uint64_t nowMs) { start(nowMs); }

    void SlotMachine::start(std::uint64_t nowMs) {
        if (m_hooks.freeBagPlaces() == 0) {
            m_hooks.message(kFull, kMessageMs);
            return;
        }
        if (freePlay() && !m_paid) {
            m_exitShown = false;
            m_hooks.ask(kFreePlay, false, [this] { confirmPayment(); }, [this] { cancelPayment(); });
            return;
        }
        if (m_startConfirmed) {
            confirmPayment();
            if (!m_startConfirmed) {
                return;
            }
        }
        if (!m_paid && !m_startConfirmed) {
            choosePayment();
            return;
        }
        m_resultShown = false;
        if (m_spinning) {
            return;
        }
        m_paid = false;
        m_askPayment = false;
        m_spinSoundPending = false;
        m_spinStartMs = nowMs;
        m_reelMs = nowMs;
        m_flashFrame = 1;
        for (int reel = 1; reel <= kReels; ++reel) {
            m_reelStopped[static_cast<std::size_t>(reel)] = false;
        }
        sendPlay();
    }

    void SlotMachine::sendPlay() {
        const std::uint8_t star = m_luckyStarOn ? static_cast<std::uint8_t>(m_luckyStarPlace) : 0;
        const std::uint8_t kind = static_cast<std::uint8_t>(m_kind);
        // packet 71/kind: kind 10 sends only the star place, the others also the way to pay
        if (m_kind == 10) { // machine kind 10 pays with points only
            m_hooks.send({71, kind, star}); // message 71 = slot machine play
        } else {
            m_hooks.send({71, kind, static_cast<std::uint8_t>(m_payKind), star}); // m_payKind: 1 points, 2 Carnie Token, 3 Fun Token
        }
    }

    void SlotMachine::choosePayment() {
        if (m_kind != 10 && freePlay()) {
            m_hooks.ask(kFreePlay, false, [this] { confirmPayment(); }, [this] { cancelPayment(); });
            return;
        }
        const int tokens = m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken);
        if (m_kind == 10) {
            m_exitShown = false;
            m_hooks.ask(pointsQuestion(), false, [this] { confirmPayment(); }, [this] { cancelPayment(); });
            return;
        }
        if (tokens != 0) {
            if (m_askPayment) {
                m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int choice) { askToPay(choice); });
            } else if (m_payKind == 2 || m_payKind == 3) { // pay kind 2 Carnie Tokens, 3 Fun Tokens (1 points)
                m_exitShown = false;
                m_hooks.ask(
                    tokensQuestion(m_hooks.countItem(m_payKind == 2 ? kCarnieToken : kFunToken)),
                    true,
                    [this] { confirmPayment(); },
                    [this] { cancelPayment(); }
                );
            } else {
                m_payKind = 1;
                m_exitShown = false;
                m_hooks.ask(pointsQuestion(), true, [this] { confirmPayment(); }, [this] { cancelPayment(); });
            }
            return;
        }
        m_payKind = 1;
        m_exitShown = false;
        m_hooks.ask(pointsQuestion(), m_kind == 19, [this] { confirmPayment(); }, [this] { cancelPayment(); });
    }

    void SlotMachine::askToPay(int choice) {
        m_exitShown = false;
        if (choice == 0) {
            m_payKind = 1;
            m_hooks.ask(pointsQuestion(), true, [this] { confirmPayment(); }, [this] { cancelPayment(); });
        } else if (choice == 1 || choice == 2) { // choice 1 Carnie Tokens, 2 Fun Tokens; pay kind is choice + 1
            m_payKind = choice + 1;
            m_hooks.ask(
                tokensQuestion(m_hooks.countItem(choice == 1 ? kCarnieToken : kFunToken)),
                true,
                [this] { confirmPayment(); },
                [this] { cancelPayment(); }
            );
        }
    }

    bool SlotMachine::hasEnoughToPlay() {
        const int kind = m_kind == 10 ? 1 : m_payKind; // pay kind: 1 points, 2 Carnie Token, 3 Fun Token
        if (kind == 1) {
            if (m_hooks.imPoints() >= pointCost() || freePlay()) {
                return true;
            }
            m_hooks.message("Please supply more points", kShortMessageMs);
            cancelPayment();
            return false;
        }
        if (m_hooks.countItem(kind == 2 ? kCarnieToken : kFunToken) >= tokenCost()) {
            return true;
        }
        m_hooks.message("Not enough carnie tokens", kShortMessageMs);
        cancelPayment();
        return false;
    }

    void SlotMachine::confirmPayment() {
        m_exitShown = true;
        m_startConfirmed = true;
        if (hasEnoughToPlay()) {
            m_paid = true;
        }
    }

    void SlotMachine::cancelPayment() {
        m_exitShown = true;
        if (m_kind != 10) {
            m_askPayment = true;
        }
        m_startConfirmed = false;
    }

    void SlotMachine::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int code = reader.u8();
        if (code == 1) { // 71/kind code 1 (server to client): prize slot, three reel targets, prize count
            const int slot = reader.u8();
            const int reel1 = reader.u8();
            const int reel2 = reader.u8();
            const int reel3 = reader.u8();
            const int count = reader.u8();
            if (reader.failed() || slot > MachineTables::kPrizes) {
                return;
            }
            m_prizeSlot = slot;
            m_prizeItem = MachineTables::slotPrizes(m_kind)[static_cast<std::size_t>(slot)];
            m_prizeKnown = true;
            m_wonPrize = slot != 0;
            m_prizeFlash = false;
            // reels are numbered 1 to 3
            m_reelTarget[1] = reel1;
            m_reelTarget[2] = reel2;
            m_reelTarget[3] = reel3;
            m_prizeCount = count;
            m_spinning = true;
            m_hooks.send(protocol::imPointsRequest(0));
            m_hooks.playSound(kPullSound);
            m_spinSoundPending = true;
        } else if (code == 2) { // 71/kind code 2 (server to client): the points are short
            m_hooks.message("Please supply more points", kMessageMs);
            m_resultShown = true;
        }
    }

    void SlotMachine::step(std::uint64_t nowMs) {
        if (m_spinning) {
            if (m_spinSoundPending) {
                m_hooks.playSound(kSpinSound);
                m_spinSoundPending = false;
            }
            if (nowMs - m_reelMs > static_cast<std::uint64_t>(m_reelIntervalMs)) {
                m_reelMs = nowMs;
                for (int reel = 1; reel <= kReels; ++reel) {
                    if (!m_reelStopped[static_cast<std::size_t>(reel)]) {
                        int& position = m_reelPos[static_cast<std::size_t>(reel)];
                        if (++position > kReelFrames) {
                            position -= kReelFrames;
                        }
                    }
                }
            }
            for (int reel = 1; reel <= kReels; ++reel) {
                if (!m_reelStopped[static_cast<std::size_t>(reel)]) {
                    if (nowMs - m_spinStartMs > MachineTables::reelStopMs(reel) &&
                        m_reelPos[static_cast<std::size_t>(reel)] == m_reelTarget[static_cast<std::size_t>(reel)]) {
                        m_reelStopped[static_cast<std::size_t>(reel)] = true;
                    }
                    if (m_reelStopped[3]) { // the third (last) reel has stopped
                        m_hooks.stopSounds();
                    }
                }
            }
        }
        if (m_reelStopped[1] && m_reelStopped[2] && m_reelStopped[3]) {
            if (!m_resultShown) {
                showObtained(true);
                m_resultShown = true;
                if (m_wonPrize) {
                    m_wonPrize = false;
                    m_prizeFlash = true;
                    m_flashStartMs = nowMs;
                    m_flashMs = nowMs;
                }
            }
            m_spinning = false;
        }
        if (m_prizeFlash) {
            if (nowMs - m_flashStartMs > kFlashMs) {
                m_prizeFlash = false;
            }
            if (nowMs - m_flashMs > kFlashStepMs) {
                if (++m_flashFrame > 2) { // the flash alternates between 2 frames
                    m_flashFrame -= 2;
                }
                m_flashMs = nowMs;
            }
            if (m_luckyStarOn && m_hooks.itemInPlace(m_luckyStarPlace) != kLuckyStar) {
                m_luckyStarOn = false;
                luckyStarClicked(false);
            }
        }
    }

    int SlotMachine::bannerSlide(std::uint64_t nowMs) {
        if (!m_spinning) {
            m_bannerStartMs = 0;
            return 0;
        }
        if (m_bannerStartMs == 0) {
            m_bannerStartMs = nowMs;
        }
        return static_cast<int>((nowMs - m_bannerStartMs) / kBannerPixelMs);
    }

}
