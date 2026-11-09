#include "client/subsystems/minigames/FortuneEgg.h"

#include "client/subsystems/minigames/MachineTables.h"
#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

#include <string>

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kMessageMs = 1000; // message display time, ms
        constexpr int kShortMessageMs = 800; // short message display time, ms
        constexpr std::uint64_t kButtonStepMs = 200; // time per button animation frame, ms
        constexpr std::uint64_t kEggStepMs = 100; // time per egg animation frame, ms
        constexpr std::uint64_t kAfterButtonMs = 240; // pause after the button animation ends, ms
        constexpr std::uint64_t kAfterEggMs = 360; // pause after the egg animation ends, ms
        constexpr int kEggSoundFrame = 10; // egg animation frame that plays the cracking sound
        const char* const kFull = "Sorry, your inventory is full.";
        const char* const kMusic = "BGM0019"; // background music name
        const char* const kPrizeSound = "sound/wav0152.wav";

    }

    FortuneEgg::FortuneEgg(
        GameHooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void FortuneEgg::open(std::uint64_t nowMs) {
        m_musicOn = false;
        m_active = false;
        m_exitShown = true;
        m_askPayment = true;
        m_paid = false;
        m_startConfirmed = false;
        m_waitingResult = false;
        m_payKind = 1; // pay kind 1: points
        m_hasPrize = false;
        m_resultShown = false;
        m_buttonAnimating = false;
        m_eggAnimating = false;
        m_eggSoundPlayed = false;
        m_buttonFrame = 3; // resting button frame
        m_eggFrame = 0;
        m_buttonMs = nowMs;
        m_eggMs = nowMs;
        m_holdUntilMs = 0;
        m_prizeCount = 0;
        m_hooks.stopMusic();
        m_hooks.send(protocol::imPointsRequest(0)); // request argument 0 (34/1 points request)
    }

    void FortuneEgg::begin() {
        m_startConfirmed = false;
        m_active = true;
        m_hooks.send(protocol::imPointsRequest(0)); // request argument 0 (34/1 points request)
    }

    void FortuneEgg::startMusicWhenActive() {
        if (m_active && !m_musicOn) {
            m_hooks.playMusic(kMusic);
            m_musicOn = true;
        }
    }

    void FortuneEgg::leave() { exitGame(); }

    void FortuneEgg::exitClicked() {
        m_active = false;
        exitGame();
    }

    void FortuneEgg::exitGame() {
        if (!m_resultShown && m_hasPrize) {
            m_hooks.message("Obtain  [" + m_hooks.itemName(m_prizeItem) + "]\xA1I", kMessageMs);
            m_hooks.playSound(kPrizeSound);
            m_resultShown = true;
            m_hasPrize = false;
        }
        m_musicOn = false;
        m_askPayment = true;
        m_paid = false;
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    std::string FortuneEgg::pointsQuestion() const {
        return "IM Points :" + std::to_string(m_hooks.imPoints()) + "\rThe points for playing the Fortune Egg once " + std::to_string(kPointsPerPlay) +
               " points\rSure to play\xA1H";
    }

    std::string FortuneEgg::tokensQuestion(int tokens) const {
        return "Rest of the tokens:" + std::to_string(tokens) + "\rThe token for playing Fortune Egg " + std::to_string(kTokensPerPlay) + "\rContinue\xA1H";
    }

    void FortuneEgg::start() {
        if (m_buttonAnimating || m_eggAnimating || m_waitingResult) {
            return;
        }
        if (m_hooks.freeBagPlaces() == 0) {
            m_hooks.message(kFull, kMessageMs);
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
        m_hooks.send({71, 6, static_cast<std::uint8_t>(m_payKind)}); // packet 71/6: u8 pay kind (1 points, 2 Carnie, 3 Fun)
        m_paid = false;
        m_askPayment = false;
        m_waitingResult = true;
    }

    void FortuneEgg::choosePayment() {
        const int tokens = m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken);
        m_exitShown = false;
        if (tokens != 0) {
            m_exitShown = true;
            if (m_askPayment) {
                m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int choice) { askToPay(choice); });
            } else if (m_payKind == 2 || m_payKind == 3) { // pay kinds 2 and 3: Carnie Tokens, Fun Tokens
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
        m_hooks.ask(pointsQuestion(), false, [this] { confirmPayment(); }, [this] { cancelPayment(); });
    }

    void FortuneEgg::askToPay(int choice) {
        m_exitShown = false;
        if (choice == 0) {
            m_payKind = 1;
            m_hooks.ask(pointsQuestion(), true, [this] { confirmPayment(); }, [this] { cancelPayment(); });
        } else if (choice == 1 || choice == 2) { // list choices 1 and 2: Carnie Token, Fun Token
            m_payKind = choice + 1; // list choice to pay kind (2 Carnie, 3 Fun)
            m_hooks.ask(
                tokensQuestion(m_hooks.countItem(choice == 1 ? kCarnieToken : kFunToken)),
                true,
                [this] { confirmPayment(); },
                [this] { cancelPayment(); }
            );
        }
    }

    bool FortuneEgg::hasEnoughToPlay() {
        if (m_payKind == 1) {
            if (m_hooks.imPoints() >= kPointsPerPlay) {
                return true;
            }
            m_hooks.message("Please supply more points", kShortMessageMs);
            cancelPayment();
            return false;
        }
        if (m_hooks.countItem(m_payKind == 2 ? kCarnieToken : kFunToken) >= kTokensPerPlay) { // pay kind 2: Carnie Tokens, else Fun Tokens
            return true;
        }
        m_hooks.message("Not enough carnie tokens", kShortMessageMs);
        cancelPayment();
        return false;
    }

    void FortuneEgg::confirmPayment() {
        m_exitShown = true;
        m_startConfirmed = true;
        if (hasEnoughToPlay()) {
            m_paid = true;
        }
    }

    void FortuneEgg::cancelPayment() {
        m_exitShown = true;
        m_askPayment = true;
        m_startConfirmed = false;
    }

    void FortuneEgg::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip the 2 byte packet header (71/6)
        const int code = reader.u8(); // result code: 1 the egg, 2 points short
        if (code == 1) { // code 1: the egg (u8 prize number, u8 count)
            const int prize = reader.u8();
            const int count = reader.u8();
            if (reader.failed() || prize < 1 || prize > MachineTables::kPrizes) {
                return;
            }
            m_hasPrize = true;
            if (!m_buttonAnimating && !m_eggAnimating) {
                m_hooks.playSound("sound/wav9914.wav");
            }
            m_buttonAnimating = true;
            m_buttonFrame = 2; // first pressed button frame
            m_prizeItem = MachineTables::eggPrizes()[static_cast<std::size_t>(prize)];
            m_prizeCount = count;
            m_waitingResult = false;
            m_hooks.send(protocol::imPointsRequest(0)); // request argument 0 (34/1 points request)
        } else if (code == 2) { // code 2: the points are short
            m_hooks.message("Please supply more points", kMessageMs);
            m_resultShown = true;
            m_waitingResult = false;
        }
    }

    void FortuneEgg::step(std::uint64_t nowMs) {
        if (nowMs < m_holdUntilMs) {
            return;
        }
        if (m_buttonAnimating && !m_eggAnimating) {
            if (nowMs - m_buttonMs > kButtonStepMs) {
                ++m_buttonFrame;
                m_buttonMs = nowMs;
            }
            if (m_buttonFrame == kButtonFrames) {
                m_buttonFrame = 0;
                m_eggAnimating = true;
                m_buttonAnimating = false;
                m_hooks.playSound("sound/wav9913.wav");
                m_holdUntilMs = nowMs + kAfterButtonMs;
                return;
            }
        }
        if (m_eggAnimating) {
            if (nowMs - m_eggMs > kEggStepMs) {
                ++m_eggFrame;
                m_eggMs = nowMs;
            }
            if (m_eggFrame == kEggFrames) {
                if (!m_resultShown && m_hasPrize) {
                    m_hooks.message("Obtain " + std::to_string(m_prizeCount) + " Quantity  [" + m_hooks.itemName(m_prizeItem) + "]\xA1I", kMessageMs);
                    m_hooks.playSound(kPrizeSound);
                    m_resultShown = true;
                    m_hasPrize = false;
                }
                m_holdUntilMs = nowMs + kAfterEggMs;
                m_eggFrame = 0;
                m_eggAnimating = false;
                m_buttonAnimating = false;
                m_eggSoundPlayed = false;
                return;
            }
            if (m_eggFrame == kEggSoundFrame && !m_eggSoundPlayed) {
                m_hooks.playSound("sound/wav9911.wav");
                m_eggSoundPlayed = true;
            }
        }
    }

}
