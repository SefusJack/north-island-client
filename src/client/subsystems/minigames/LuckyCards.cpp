#include "client/subsystems/minigames/LuckyCards.h"

#include "core/ByteReader.h"
#include "protocol/MallMessages.h"
#include "protocol/MessageId.h"

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kMessageMs = 1300; // short message display time, ms
        constexpr int kRewardMessageMs = 3500; // reward message display time, ms
        constexpr int kMostFlips = 4; // most cards turned in one play (two tries of two cards)
        constexpr int kFlipStep = 50; // card flip animation: px each side of the card is narrowed per tick
        constexpr int kFlipLimit = 150; // card flip animation midpoint: side offset at which the flip turns back, px
        constexpr std::uint64_t kJudgeMs = 1000; // wait after two turned cards before judging the pair, ms
        constexpr std::uint64_t kClearMs = 2000; // wait before the turned cards are cleared, ms
        const char* const kFlipSound = "sound/wav0154.wav"; // card turn and deal sound
        const char* const kPairSound = "sound/Wav1610.wav"; // matching pair sound
        const char* const kMissSound = "sound/Wav1609.wav"; // no match sound
        const char* const kPrizeSound = "sound/wav0152.wav"; // prize sound
        const char* const kMusic = "BGM0019"; // background music name
        const char* const kNoRoom = "Item slots are not enough\rplease reserve 2 slots for rewards.";

        /// @brief Builds a 57/9 poke game request.
        /// @param code Sub code of the request.
        /// @return The packet bytes.
        core::Bytes pokeRequest(int code) { return {57, 9, static_cast<std::uint8_t>(code)}; } // packet 57, game 9 (Lucky Cards)

    }

    LuckyCards::LuckyCards(
        GameHooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void LuckyCards::open(std::uint64_t nowMs) {
        setCardsVisible(false);
        m_lastTickMs = nowMs;
        m_tick = 0;
        m_playing = false;
        m_payKind = 0;
        m_quitting = false;
        m_flipLocked = false;
        m_mouseDownSeen = false;
        m_cardsDealt = false;
        m_flipOffset = 0;
        m_flippingCard = 0;
        m_flipPending = false;
        m_pairMatched = false;
        m_pairChecked = false;
        m_flipTickMs = 0;
        m_flippedValues = {};
        m_flipAnimating = false;
        m_flipHalfDone = false;
        reset();
        m_hooks.send(protocol::imPointsRequest(0));
    }

    void LuckyCards::reset() {
        m_tick = 0;
        m_playing = false;
        m_flipLocked = false;
        m_values = {};
        m_flippedCount = 0;
        m_cardCount = 0;
    }

    void LuckyCards::begin() { setCardsVisible(false); }

    void LuckyCards::setCardsVisible(bool visible) { m_visible.fill(visible); }

    void LuckyCards::receive(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 2);
        const int code = reader.u8(); // 57/9 sub code: 3 card turned, 4 faces shown, 6 deal
        switch (code) {
            case 3: { // 57/9/3: a card turned (u8 card, u8 value, u8 turned count)
                const int card = reader.u8();
                const int value = reader.u8();
                const int flipped = reader.u8();
                if (reader.failed() || card < 1 || card > kCards || flipped < 1 || flipped > kMostFlips) {
                    return;
                }
                m_values[static_cast<std::size_t>(card)] = value;
                m_flippedCount = flipped;
                m_flippedValues[static_cast<std::size_t>(flipped)] = value;
                if (m_flippedCount == 2 || m_flippedCount == kMostFlips) {
                    m_flipPending = true;
                    m_flipTickMs = nowMs;
                    checkPair(flipped);
                    if (m_flippedCount == kMostFlips) {
                        setCardsVisible(false);
                    }
                }
                if (m_flippedCount != 2) {
                    m_flipLocked = false;
                }
                break;
            }
            case 4: // 57/9/4: the server shows every face (8 values)
                if (m_flippedCount == kMostFlips) {
                    setCardsVisible(false);
                }
                for (int card = 1; card <= kCards; ++card) {
                    m_values[static_cast<std::size_t>(card)] = reader.u8();
                }
                break;
            case 6: // 57/9/6: the server deals, the play starts
                reset();
                m_playing = true;
                m_cardsDealt = true;
                m_flipLocked = true;
                m_hooks.playMusic(kMusic);
                break;
            default: // other sub ids ignored
                break;
        }
    }

    void LuckyCards::checkPair(int flipped) {
        const int first = flipped == 2 ? 1 : 3; // the first try compares turned cards 1 and 2, the second try 3 and 4
        const int second = first + 1;
        const int one = m_flippedValues[static_cast<std::size_t>(first)];
        const int two = m_flippedValues[static_cast<std::size_t>(second)];
        if (one == two && one != 0) {
            m_pairMatched = true;
        } else if (one != two && one != 0) {
            m_pairMatched = false;
        }
        m_flippedValues[static_cast<std::size_t>(first)] = 0;
        m_flippedValues[static_cast<std::size_t>(second)] = 0;
        m_pairChecked = true;
    }

    void LuckyCards::soundResult() {
        if (m_pairChecked) {
            m_hooks.playSound(m_pairMatched ? kPairSound : kMissSound);
        }
        m_pairChecked = false;
    }

    void LuckyCards::receiveMachine(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        const int code = reader.u8();
        if (code == 2) { // 71/16/2: points are short
            m_playing = false;
            m_hooks.requestMessage("Please supply more points", kMessageMs);
        } else if (code == 4 && m_quitting) { // 71/16/4: prize for leaving (i32 item id)
            const std::int32_t itemId = reader.i32();
            if (reader.failed()) {
                return;
            }
            m_hooks.requestMessage(std::string("The result is\xA1G\rGot an honorable prize [ ") + m_hooks.itemName(itemId) + " ]", kRewardMessageMs);
            m_hooks.playSound(kPrizeSound);
            exitGame();
        }
    }

    void LuckyCards::update(std::uint64_t nowMs) {
        constexpr int kMostCatchUp = 10; // most ticks run in one update, to avoid a spiral after a stall
        m_previousFlipOffset = m_flipOffset;
        m_updateMs = nowMs;
        for (int frames = 0; nowMs >= m_lastTickMs + kTickMs && frames < kMostCatchUp; ++frames) {
            m_lastTickMs += kTickMs;
            m_tick = m_tick > 10000 ? 0 : m_tick + 1; // tick counter wraps after 10000
            if (m_cardsDealt) {
                deal();
            }
            flipAnimation();
            judgeFlips(nowMs);
        }
        if (nowMs >= m_lastTickMs + kTickMs * kMostCatchUp) {
            m_lastTickMs = nowMs;
        }
    }

    void LuckyCards::deal() {
        setCardsVisible(false);
        m_values = {};
        if (m_tick % 10 != 0) { // a card every ten frames
            return;
        }
        if (m_cardCount < kCards) {
            ++m_cardCount;
            m_hooks.playSound(kFlipSound);
        } else if (m_cardCount == kCards) {
            setCardsVisible(true);
            m_cardsDealt = false;
            m_flipLocked = false;
        }
    }

    void LuckyCards::flipAnimation() {
        if (!m_flipAnimating) {
            return;
        }
        if (m_flipOffset < kFlipLimit && !m_flipHalfDone) {
            m_flipOffset += kFlipStep;
        } else if (m_flipOffset == kFlipLimit && !m_flipHalfDone) {
            m_flipHalfDone = true;
            m_hooks.playSound(kFlipSound);
        } else if (m_flipHalfDone && m_flipOffset > 0) {
            m_flipOffset -= kFlipStep;
        } else if (m_flipOffset == 0) {
            m_flipAnimating = false;
            m_flipHalfDone = false;
        }
    }

    void LuckyCards::judgeFlips(std::uint64_t nowMs) {
        if (nowMs - m_flipTickMs > kJudgeMs && m_flippedCount == 2 && m_flipPending) {
            if (m_pairMatched) {
                setCardsVisible(false);
            } else {
                setCardsVisible(true);
                m_flipPending = false;
                m_flipLocked = false;
            }
        }
        if (nowMs - m_flipTickMs > kClearMs) {
            if (m_flippedCount == 2 && m_flipPending) {
                if (!m_pairMatched) {
                    setCardsVisible(true);
                } else {
                    m_cardCount = 0;
                    m_cardsDealt = true;
                    m_flipLocked = true;
                }
                m_flipPending = false;
            }
            if (m_flippedCount == kMostFlips && m_flipPending) {
                m_flipPending = false;
                m_playing = false;
                m_flippedCount = 0;
                m_hooks.send(pokeRequest(4)); // 57/9/4: ask the server to show every face
            }
        }
    }

    void LuckyCards::tableClicked() {
        if (!m_playing && !m_cardsDealt) {
            m_mouseDownSeen = true;
            choosePayment();
        }
    }

    void LuckyCards::cardClicked(int card) {
        if (card < 1 || card > kCards || !m_visible[static_cast<std::size_t>(card)]) {
            return;
        }
        if (m_flipLocked || m_flipAnimating || m_flipHalfDone) {
            return;
        }
        if (m_flippedCount == 2) {
            m_values = {};
        }
        if (m_flipPending || m_flippedCount >= kMostFlips) {
            return;
        }
        m_flipAnimating = true;
        m_flippingCard = card;
        m_visible[static_cast<std::size_t>(card)] = false;
        m_flipLocked = true;
        m_hooks.send({57, 9, 3, static_cast<std::uint8_t>(card)}); // 57/9/3: turn this card
    }

    bool LuckyCards::hasTwoFreeBagPlaces() const { return m_hooks.freeBagPlaces() >= 2; }

    void LuckyCards::choosePayment() {
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        const int tokens = m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken);
        if (tokens != 0) {
            m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int choice) { askToPay(choice); });
            return;
        }
        if (hasTwoFreeBagPlaces()) {
            m_hooks
                .ask(points + "\rDouble Machine requires " + std::to_string(kPointsPerPlay) + " Points!\r Sure to continue?", false, [this] { pay(1); }, [] {});
        } else {
            m_hooks.message(kNoRoom, kMessageMs);
        }
    }

    void LuckyCards::askToPay(int choice) {
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        if (!hasTwoFreeBagPlaces()) {
            m_hooks.message(kNoRoom, kMessageMs);
            return;
        }
        switch (choice) {
            case 0: // pay with points
                m_hooks.ask(
                    points + "\rDouble Machine requires " + std::to_string(kPointsPerPlay) + " Points!\r Sure to continue?",
                    true,
                    [this] { pay(1); }, // pay kind 1: points
                    [] {}
                );
                break;
            case 1: // pay with Carnie Tokens
                m_hooks.ask(
                    "Rest of the tokens:" + std::to_string(m_hooks.countItem(kCarnieToken)) + "\rDouble machine requires one flying coin" +
                        std::to_string(kTokensPerPlay) + "\rContinue\xA1H",
                    true,
                    [this] { pay(2); }, // pay kind 2: Carnie Tokens
                    [] {}
                );
                break;
            case 2: // pay with Fun Tokens
                m_hooks.ask(
                    "Rest of the tokens:" + std::to_string(m_hooks.countItem(kFunToken)) + "\rDouble machine requires one flying coin" +
                        std::to_string(kTokensPerPlay) + "\rContinue\xA1H",
                    true,
                    [this] { pay(3); }, // pay kind 3: Fun Tokens
                    [] {}
                );
                break;
            default: // cancelled
                break;
        }
    }

    void LuckyCards::pay(int kind) {
        if (kind == 1) {
            if (m_hooks.imPoints() < kPointsPerPlay) {
                m_hooks.message("Please supply more points", kMessageMs);
                return;
            }
        } else if (m_hooks.countItem(kind == 2 ? kCarnieToken : kFunToken) < kTokensPerPlay) {
            m_hooks.message("Not enough carnie tokens", kMessageMs);
            return;
        }
        if (!hasTwoFreeBagPlaces()) {
            m_hooks.message(kNoRoom, kMessageMs);
            return;
        }
        m_payKind = kind;
        m_hooks.send({71, 16, static_cast<std::uint8_t>(kind)}); // packet 71, machine kind 16, pay kind: play request
    }

    void LuckyCards::quitClicked() {
        const std::string text = m_payKind == 1 ? "Has the chance to trun over the card\rwant to leave\xA1H\r ponints deducted"
                                                : "Has the chance to trun over the card\rWant to leave\xA1H\r flying coin deducted ";
        if (!m_playing) {
            exitGame();
            return;
        }
        if (m_flippedCount <= 1) {
            m_hooks.ask(text, false, [this] { exitGame(); }, [] {});
        } else if (m_flippedCount <= 3) {
            m_hooks.ask(text, false, [this] { quitGame(); }, [] {});
        }
    }

    void LuckyCards::quitGame() {
        m_quitting = true;
        exitGame();
    }

    void LuckyCards::exitGame() {
        reset();
        m_hooks.stopMusic();
        m_hooks.endGame();
        m_hooks.send(pokeRequest(2)); // 57/9/2: leave the game
    }

}
