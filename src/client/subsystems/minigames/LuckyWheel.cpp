#include "client/subsystems/minigames/LuckyWheel.h"

#include "client/subsystems/minigames/WheelPrizes.h"
#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

#include <algorithm>
#include <random>

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kMessageMs = 1300; // how long a notice message shows, ms
        constexpr int kRounds = 3; // full laps of the 20 prizes a spin runs
        const char* const kFull = "Item bar is full\rNot enough space for the rewards";
        const char* const kWheelMusic = "BGM0019";
        constexpr int kMostSteps = kRounds * 20 + 10; // upper bound of spin steps: 3 laps (20 prizes each) + 10
        constexpr int kLeastSteps = (kRounds - 1) * 20 + 10; // lower bound of spin steps: 2 laps + 10

        /// @brief Builds a 57/12 wheel request.
        /// @param value Sub code of the request.
        /// @return The packet bytes.
        core::Bytes wheelRequest(int value) { return {57, 12, static_cast<std::uint8_t>(value)}; } // packet 57, game 12 (Lucky Wheel)

    }

    LuckyWheel::LuckyWheel(
        const formats::ItemTable& items,
        Hooks hooks
    )
        : m_items(items),
          m_hooks(std::move(hooks)) {}

    void LuckyWheel::setFormOrigin(core::Point origin) {
        // prize places: top row 1-6, right column 7-11, bottom row 12-16, left column 17-20; 54 px between prizes, the rest are form offsets
        for (int i = 1; i <= kPrizes; ++i) {
            core::Point at;
            if (i < 7) {
                at = {(i - 1) * 54 + 65 + origin.x, origin.y + 74};
            } else if (i < 12) {
                at = {origin.x + 335, (i - 7) * 54 + 128 + origin.y};
            } else if (i < 17) {
                at = {281 - (i - 12) * 54 + origin.x, origin.y + 344};
            } else {
                at = {origin.x + 65, 290 - (i - 17) * 54 + origin.y};
            }
            m_prizes[static_cast<std::size_t>(i)].position = at;
        }
    }

    void LuckyWheel::reset(std::uint64_t nowMs) {
        for (TopPrize& top : m_panel) {
            top = {};
        }
        for (Prize& prize : m_prizes) {
            prize.itemId = 0;
            prize.count = 0;
            prize.name.clear();
        }
        m_spinMs = nowMs;
        m_blinkMs = nowMs;
        m_spinAccelSteps = 0;
        m_spinIntervalMs = 500; // first spin step interval in ms
        m_blinkIntervalMs = 300; // blink interval in ms
        m_lightsActive = false;
        m_lightMs = nowMs;
        m_lightIntervalMs = 0;
        m_lightMask = 0;
    }

    void LuckyWheel::open(std::uint64_t nowMs) {
        reset(nowMs);
        m_state = 0;
        m_refreshShown = m_doubleShown = m_startShown = m_exitShown = false;
    }

    void LuckyWheel::begin(std::uint64_t nowMs) {
        m_spinPos = static_cast<int>(m_random() % 20) + 1; // a random one of the 20 prizes
        reset(nowMs);
        m_state = 0;
        m_payKind = 0;
        m_paid = false;
        m_lightsPending = false;
        m_spinStepsLeft = 0;
        m_refreshShown = m_doubleShown = m_startShown = m_exitShown = true;
        m_doublePending = false;
        m_doubleAsked = false;
        m_highlightRow = 0;
        m_resultAnimStep = 1;
        m_payMode = 0;
        m_hooks.send(protocol::imPointsRequest(0));
    }

    void LuckyWheel::receiveList(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 2);
        switch (reader.u8()) {
            case 1: setup(payload, false, nowMs); break; // 57/12/1: prize list as codes
            case 2: { // 57/12/2: double (reduplication) result
                const int outcome = reader.u8();
                if (outcome == 0) {
                    m_doubleShown = true;
                    m_hooks.message("Don't have enough space, cannot use reduplication", kMessageMs); // outcome 0: no space; 1: accepted
                } else if (outcome == 1) {
                    m_doublePending = true;
                }
                break;
            }
            case 3: // 57/12/3: server busy notice
                if (reader.u8() == 1) {
                    m_hooks.message("The server is busy", 1500); // message time in ms
                }
                break;
            case 5: setup(payload, true, nowMs); break; // 57/12/5: prize list as explicit items
            default: break; // other sub ids ignored
        }
    }

    void LuckyWheel::setup(const core::Bytes& payload, bool explicitItems, std::uint64_t nowMs) {
        if (!explicitItems && payload.size() < 22) { // 2 header bytes + 20 prize codes
            return;
        }
        reset(nowMs);
        core::ByteReader reader(payload, 3);
        // panel places: first prize 1, second prizes from 2, third prizes from 4
        int secondSlot = 2;
        int thirdSlot = 4;
        for (int i = 1; i <= kPrizes; ++i) {
            const int code = reader.u8();
            int tier = 0;
            int itemId = 0;
            int count = 0;
            if (explicitItems) {
                itemId = reader.u16();
                count = reader.u8();
                tier = code >= 1 && code <= 30   ? 1
                       : code > 30 && code <= 60 ? 2
                       : code > 60 && code <= 90 ? 3
                                                 : 0; // prize code ranges 1-30, 31-60, 61-90 give tiers 1 to 3
            } else {
                const WheelPrize prize = wheelPrizeOf(code);
                itemId = prize.itemId;
                count = prize.count;
                tier = prize.tier;
            }
            if (reader.failed()) {
                return;
            }
            Prize& prize = m_prizes[static_cast<std::size_t>(i)];
            prize.itemId = itemId;
            prize.count = count;
            const auto item = m_items.find(itemId);
            prize.name = item ? item->name : std::string();
            if (tier == 0) {
                continue;
            }
            int slot = tier == 1 ? 1 : tier == 2 ? secondSlot++ : thirdSlot++;
            if (slot > kPanelSlots) {
                continue;
            }
            static const char* const kTierNames[] = {"", "First Prize: ", "Second Prize: ", "Third Prize: "};
            TopPrize& top = m_panel[static_cast<std::size_t>(slot)];
            top.prizeIndex = i;
            top.itemId = itemId;
            top.count = explicitItems ? count : 0;
            top.tier = tier;
            top.text = std::string(explicitItems ? "" : kTierNames[tier]) + prize.name;
            m_hooks.frameEffect(slot, prize.position, tier);
        }
        m_state = 1;
        m_highlightRow = 0;
    }

    void LuckyWheel::receiveError(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        if (reader.u8() != 2) { // 71/21/2: play error (not enough points or tokens)
            return;
        }
        m_hooks.requestMessage(m_payKind == 1 ? "Please supply more points" : "Not enough carnie tokens", kMessageMs);
        m_refreshShown = m_doubleShown = m_startShown = true;
        m_paid = false;
        m_payKind = 0;
        m_state = 1;
        m_doublePending = false;
    }

    void LuckyWheel::receiveResult(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 3);
        const int prizeIndex = reader.u8();
        const int outcome = reader.u8();
        if (reader.failed() || prizeIndex < 1 || prizeIndex > kPrizes) {
            return;
        }
        m_spinIntervalMs = 500; // first spin step interval in ms
        m_blinkIntervalMs = 300; // blink interval in ms
        m_spinAccelSteps = 0;
        // 20 is the number of prizes on the wheel (one round)
        int steps = prizeIndex - m_spinPos + kRounds * 20;
        if (steps > kMostSteps) {
            steps -= 20;
        }
        if (steps < kLeastSteps) {
            steps += 20;
        }
        m_spinStepsLeft = steps;
        m_state = 2;
        m_wonName = m_prizes[static_cast<std::size_t>(prizeIndex)].name;
        m_wonCount = outcome;
        m_payKind = 0;
        m_winSound = "sound/wav0152.wav"; // plain win sound
        m_lightsPending = false;
        m_highlightRow = 0;
        for (int slot = 1; slot <= kPanelSlots; ++slot) {
            if (m_panel[static_cast<std::size_t>(slot)].itemId == m_prizes[static_cast<std::size_t>(prizeIndex)].itemId) {
                m_lightsPending = true;
                m_winSound = "sound/wav0150.wav"; // win sound when the prize is one of the top-prize panel items
                // screen positions of the three win lights
                startLight(0, {200, 300}, nowMs);
                startLight(1, {400, 150}, nowMs);
                startLight(2, {600, 450}, nowMs);
                m_lightIntervalMs = 0;
                m_resultRow = slot;
                break;
            }
        }
    }

    void LuckyWheel::startLight(int slot, core::Point position, std::uint64_t nowMs) {
        WinLight& light = m_lights[static_cast<std::size_t>(slot)];
        light = {};
        light.position = position;
        light.frameMs = nowMs;
    }

    void LuckyWheel::advanceLight(int slot, std::uint64_t nowMs) {
        WinLight& light = m_lights[static_cast<std::size_t>(slot)];
        if (light.finished || nowMs - light.frameMs <= static_cast<std::uint64_t>(kLightIntervalMs)) {
            return;
        }
        light.frameMs = nowMs;
        if (light.frame + 1 >= kLightFrames) {
            light.finished = true;
            return;
        }
        ++light.frame;
        if (light.frame == 1) { // the second frame plays the sound
            m_hooks.playSound("sound/SEB0051.wav"); // win light sound
        }
    }

    void LuckyWheel::endLightsWhenFinished() {
        if (m_lights[0].finished && m_lights[1].finished && m_lights[2].finished) {
            m_lights = {};
            m_lightMask = 0;
            m_lightsActive = false;
        }
    }

    bool LuckyWheel::hasEmptyPlace() const { return m_hooks.freeBagPlaces() > 0; }

    bool LuckyWheel::canPlay() {
        if (m_payKind == 1) {
            const int cost = m_doublePending ? kPointsPerSpin * 2 : kPointsPerSpin;
            if (m_hooks.imPoints() < cost) {
                m_hooks.message("Please supply more points", kMessageMs);
                return false;
            }
        } else if (m_payKind == 2 || m_payKind == 3) { // pay kinds 2 and 3: Carnie and Fun Tokens
            const int cost = m_doublePending ? kTokensPerSpin * 2 : kTokensPerSpin;
            if (m_hooks.countItem(m_payKind == 2 ? kCarnieToken : kFunToken) < cost) {
                m_hooks.message("Not enough carnie tokens", kMessageMs);
                return false;
            }
        }
        const int spins = m_doublePending ? 2 : 1;
        if (m_hooks.freeBagPlaces() < spins) {
            m_hooks.message(kFull, kMessageMs);
            return false;
        }
        return true;
    }

    void LuckyWheel::choosePayment() {
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        const int tickets = m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken);
        if (tickets != 0) {
            m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int kind) { askToPay(kind); }); // \xA1H = Big5 full-width "?"
            return;
        }
        if (hasEmptyPlace()) {
            m_payMode = 1; // pay mode 1: points, 2: tokens
            m_hooks.ask(
                points + "\rCost to play the Happy Wheel once " + std::to_string(kPointsPerSpin) + " Points!\r Sure to continue?",
                false,
                [this] { pay(1); },
                [] {}
            );
        } else {
            m_hooks.message(kFull, kMessageMs);
        }
    }

    void LuckyWheel::askToPay(int kind) {
        if (!hasEmptyPlace()) {
            m_hooks.message(kFull, kMessageMs);
            return;
        }
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        switch (kind) {
            case 0: // pay with points
                m_hooks.ask(
                    points + "\rCost to play the Happy Wheel once " + std::to_string(kPointsPerSpin) + " Points!\r Sure to continue?",
                    true,
                    [this] { pay(1); },
                    [] {}
                );
                break;
            case 1: // pay with Carnie Tokens
                m_hooks.ask(
                    "Rest of the tokens:" + std::to_string(m_hooks.countItem(kCarnieToken)) + "\rCost to play the Happy Wheel once " +
                        std::to_string(kTokensPerSpin) + "\rContinue\xA1H",
                    true,
                    [this] { pay(2); },
                    [] {}
                );
                break;
            case 2: // pay with Fun Tokens
                m_hooks.ask(
                    "Rest of the tokens:" + std::to_string(m_hooks.countItem(kFunToken)) + "\rCost to play the Happy Wheel once " +
                        std::to_string(kTokensPerSpin) + "\rContinue\xA1H",
                    true,
                    [this] { pay(3); },
                    [] {}
                );
                break;
            default: break; // cancelled
        }
    }

    void LuckyWheel::pay(int kind) {
        if (kind == 1) {
            if (m_hooks.imPoints() < kPointsPerSpin) {
                m_hooks.message("Please supply more points", kMessageMs);
                return;
            }
        } else {
            if (m_hooks.countItem(kind == 2 ? kCarnieToken : kFunToken) < kTokensPerSpin) {
                m_hooks.message("Not enough carnie tokens", kMessageMs);
                return;
            }
        }
        if (hasEmptyPlace()) {
            m_payKind = kind;
            m_hooks.playMusic(kWheelMusic);
            m_paid = true;
        } else {
            m_hooks.message(kFull, kMessageMs);
        }
        if (m_payKind != 0) {
            m_hooks.send(wheelRequest(kind == 1 ? 3 : 4)); // 57/12/3: paid with points, 57/12/4: paid with tokens
        }
        m_payMode = kind == 1 ? 1 : 2;
        if (kind != 1) {
            m_doubleShown = false;
        }
    }

    void LuckyWheel::refreshClicked() {
        if (!m_refreshShown) {
            return;
        }
        if (!m_paid) {
            choosePayment();
        } else if (canPlay()) {
            m_hooks.send(wheelRequest(1)); // 57/12/1: refresh the prize group
        }
    }

    void LuckyWheel::doubleClicked() {
        if (!m_doubleShown) {
            return;
        }
        if (!m_paid) {
            choosePayment();
        } else if (m_payMode == 1) {
            if (m_hooks.imPoints() < kPointsPerSpin * 2) {
                m_hooks.message("Please supply more points", kMessageMs);
            } else if (!m_doubleAsked) {
                m_hooks.ask("2X points, 2X rewards\rWanna use?", false, [this] { confirmDouble(); }, [] {});
            } else {
                confirmDouble();
            }
        }
    }

    void LuckyWheel::confirmDouble() {
        m_doublePending = true;
        if (!canPlay()) {
            m_doublePending = false;
            return;
        }
        m_doubleShown = false;
        m_hooks.send(wheelRequest(2)); // 57/12/2: ask to double
        m_doubleAsked = true;
    }

    void LuckyWheel::startClicked() {
        if (!m_startShown) {
            return;
        }
        if (!m_paid) {
            choosePayment();
        } else if (canPlay()) {
            m_startShown = m_refreshShown = m_doubleShown = false;
            m_hooks.send({71, 21, static_cast<std::uint8_t>(m_payKind)}); // packet 71, machine kind 21, pay kind: spin request
        }
    }

    void LuckyWheel::exitClicked() {
        m_hooks.stopMusic();
        m_hooks.endGame();
        if (m_state == 2) {
            m_hooks.message("Congratulations. You have obtained [" + m_wonName + "] X" + std::to_string(m_wonCount), kMessageMs);
            m_hooks.playSound(m_winSound);
            m_state = 1;
        }
    }

    void LuckyWheel::spinStep(std::uint64_t nowMs) {
        if (m_state != 2) {
            return;
        }
        if (nowMs - m_spinMs < static_cast<std::uint64_t>(std::max(0, m_spinIntervalMs))) {
            return;
        }
        m_hooks.playSound("sound/wav1603.wav"); // spin tick sound
        m_spinMs = nowMs;
        // the highlight cycles over 6 panel rows; the first 5 steps speed up by 100 ms, the last 5 slow down by 100 ms; the spin cycles over 20 prizes
        m_highlightRow = m_highlightRow + 1 > 6 ? 1 : m_highlightRow + 1;
        if (m_spinAccelSteps <= 4) {
            m_spinIntervalMs = std::max(0, m_spinIntervalMs - 100);
            ++m_spinAccelSteps;
        } else if (m_spinStepsLeft != 0 && m_spinStepsLeft <= 5) {
            m_spinIntervalMs += 100;
        }
        m_spinPos = m_spinPos + 1 > 20 ? 1 : m_spinPos + 1;
        m_resultAnimStep = m_resultAnimStep + 1 > 6 ? 1 : m_resultAnimStep + 1;
        if (m_spinStepsLeft > 0) {
            --m_spinStepsLeft;
            if (m_spinStepsLeft == 0) {
                m_state = 1;
                m_startShown = m_refreshShown = m_doubleShown = true;
                m_paid = false;
                m_doublePending = false;
                m_hooks.message("Congratulations. You have obtained [" + m_wonName + "] X" + std::to_string(m_wonCount), kMessageMs);
                m_hooks.playSound(m_winSound);
                m_highlightRow = 0;
                if (m_lightsPending) {
                    m_lightsActive = true;
                    m_lightsPending = false;
                    m_highlightRow = m_resultRow;
                }
            }
        }
    }

    void LuckyWheel::blinkStep(std::uint64_t nowMs) {
        if (nowMs - m_blinkMs >= static_cast<std::uint64_t>(m_blinkIntervalMs)) {
            m_blinkMs = nowMs;
            m_blinkPhase = m_blinkPhase + 1 > 1 ? 0 : m_blinkPhase + 1;
        }
    }

    void LuckyWheel::lightsStep(std::uint64_t nowMs) {
        if (!m_lightsActive) {
            return;
        }
        // light mask bits: 1, 3, 7 = first, first two, all three lights; 200 ms more between lights each time
        if (m_lightMask != 7 && nowMs - m_lightMs >= static_cast<std::uint64_t>(m_lightIntervalMs)) {
            m_lightMs = nowMs;
            m_lightIntervalMs += 200;
            if (m_lightMask == 0) {
                m_lightMask = 1;
            } else if (m_lightMask == 1) {
                m_lightMask = 3;
            } else if (m_lightMask == 3) {
                m_lightMask = 7;
            }
        }
    }

}
