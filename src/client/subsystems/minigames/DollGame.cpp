#include "client/subsystems/minigames/DollGame.h"

#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

namespace client {

    namespace {

        constexpr int kCarnieToken = 34007; // item 34007: Carnie Token
        constexpr int kFunToken = 34008; // item 34008: Fun Token
        constexpr int kPointsCost = 45; // IM Points one play costs
        constexpr int kTokenCost = 9; // tokens one play costs
        constexpr int kMessageMs = 1300; // short message display time, ms
        constexpr int kResultMessageMs = 2500; // catch result message display time, ms
        constexpr int kExitResultMessageMs = 4500; // result message shown on quitting, ms
        constexpr int kKinds = 7; // number of doll kinds (kDollNpcIds[1..7])
        constexpr int kBodyChangedKind = 6; // doll kind 6 (NPC 17046 Chicken) gets its body colours replaced
        constexpr std::int32_t kBodyChangedColours = 444670526; // packed colour set (0x1A81223E) replacing colour 0 of that doll
        constexpr int kFacingLeft = 11; // NPC sprite direction 11: facing left
        constexpr int kFacingRight = 13; // NPC sprite direction 13: facing right
        constexpr int kHeldFacingLeft = 30; // NPC sprite direction 30: held by the claw, left
        constexpr int kHeldFacingRight = 31; // NPC sprite direction 31: held by the claw, right
        constexpr int kFloorBottom = 450; // y of the hole at the bottom; dolls at or below it are taken, px
        constexpr int kCatchLimitY = 400; // dolls lower than this y cannot be caught, px
        constexpr int kCaughtFrame = 3; // claw grab animation frame after which the doll takes its held pose
        constexpr int kLiftPerFrame = 21; // px the held doll rises per lift frame
        constexpr int kDropStep = 15; // px the held doll falls per frame into the hole
        constexpr int kClampRise = 24; // px the held doll is raised when clamped to the claw
        constexpr int kCarryStep = 3; // px the carried doll moves per frame
        const char* const kMusic = "BGM0019"; // background music name
        const char* const kFull = "Item bar is full\rNot enough space for the rewards";
        // NPC id per doll kind (index 0 unused): 1 Monkey, 2 Lala, 3 Persian Cat, 4 Shiba inu, 5 Rice Chicken, 6 Chicken, 7 Rhesus Monkey
        constexpr int kDollNpcIds[kKinds + 1] = {0, 17162, 11006, 11000, 11003, 17042, 17046, 17165};

    }

    DollGame::DollGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {
        for (NpcDoll& doll : m_dolls) {
            chooseKind(doll);
        }
    }

    void DollGame::chooseKind(NpcDoll& doll) {
        doll.kindIdx = random(kKinds) + 1;
        if (const auto info = m_hooks.npcInfo(kDollNpcIds[doll.kindIdx])) {
            doll.npc = *info;
        }
        if (doll.kindIdx == kBodyChangedKind) {
            doll.npc.colors[0] = kBodyChangedColours;
        }
        doll.direction = random(2) == 0 ? kFacingLeft : kFacingRight; // coin flip between two facings
    }

    core::Size DollGame::spriteSizeOf(int doll) const {
        const NpcDoll& item = m_dolls[static_cast<std::size_t>(doll)];
        return m_hooks.spriteSize(item.npc, item.direction);
    }

    void DollGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_formShown = false;
        m_catchRequestSent = false;
        m_paid = false;
        m_confirmed = false;
        m_payKind = 0;
        m_dollText = "Get rewards\xA1G";
        m_catcher.reset();
        scatterDolls();
        m_hooks.stopMusic();
        m_hooks.send(protocol::imPointsRequest(0)); // request argument 0 (34/1 points request)
    }

    void DollGame::scatterDolls() {
        for (std::size_t i = 0; i < m_dolls.size(); ++i) {
            const core::Size size = spriteSizeOf(static_cast<int>(i));
            // floor area: x range 250 wide starting at 230 + 106, y band of 90/4 starting at 290; 7 and 5 are fixed margins of the original
            m_dolls[i].screen.x = random(250 - size.width / 2 - 7) + 230 + 106;
            m_dolls[i].screen.y = random(90) / 4 + 290 + size.height - 5; // y band: random 0..89 / 4 from y 290, plus sprite height
        }
        sortDollsByY();
    }

    void DollGame::sortDollsByY() {
        for (std::size_t j = 0; j < m_dolls.size(); ++j) {
            for (std::size_t i = 0; i < m_dolls.size(); ++i) {
                if (m_dolls[j].screen.y < m_dolls[i].screen.y) {
                    std::swap(m_dolls[j].screen, m_dolls[i].screen);
                }
            }
        }
    }

    void DollGame::randomizeDolls() {
        for (NpcDoll& doll : m_dolls) {
            chooseKind(doll);
        }
        scatterDolls();
    }

    void DollGame::resetDollsIfAllTaken() {
        for (const NpcDoll& doll : m_dolls) {
            if (doll.screen.y < kFloorBottom) {
                return;
            }
        }
        randomizeDolls();
    }

    int DollGame::leftEdge(int doll) const {
        if (doll < 0) {
            return 0;
        }
        const NpcDoll& item = m_dolls[static_cast<std::size_t>(doll)];
        const int width = spriteSizeOf(doll).width;
        if (item.direction == kFacingLeft) {
            if (item.kindIdx == 1) { // doll kind 1: edge inset a quarter of the width
                return item.screen.x - width + width / 4; // quarter of the width
            }
            if (item.kindIdx >= 2 && item.kindIdx <= 4) { // doll kinds 2 to 4: edge inset a third of the width
                return item.screen.x - width + width / 3; // third of the width
            }
        }
        return item.screen.x - width / 2;
    }

    int DollGame::rightEdge(int doll) const {
        if (doll < 0) {
            return 0;
        }
        const NpcDoll& item = m_dolls[static_cast<std::size_t>(doll)];
        const int width = spriteSizeOf(doll).width;
        if (item.direction == kFacingRight) {
            if (item.kindIdx == 1) { // doll kind 1: edge inset a quarter of the width
                return width + item.screen.x - width / 4; // quarter of the width
            }
            if (item.kindIdx >= 2 && item.kindIdx <= 4) { // doll kinds 2 to 4: edge inset a third of the width
                return width + item.screen.x - width / 3; // third of the width
            }
        }
        return width / 2 + item.screen.x;
    }

    void DollGame::shiftX(int doll, int delta) {
        if (doll != -1) {
            m_dolls[static_cast<std::size_t>(doll)].screen.x -= delta;
        }
    }

    void DollGame::shiftY(int doll, int delta) {
        if (doll != -1) {
            m_dolls[static_cast<std::size_t>(doll)].screen.y -= delta;
        }
    }

    void DollGame::dropTowardY(int doll, int targetY) {
        core::Point& at = m_dolls[static_cast<std::size_t>(doll)].screen;
        at.y = at.y + kCarryStep < targetY ? at.y + kCarryStep : targetY;
    }

    void DollGame::buttonDown(ClawCatcher::State state) {
        if (!m_paid && !m_confirmed) {
            chooseDollPayment();
        } else {
            m_catcher.setMoveState(state);
        }
    }

    void DollGame::buttonUp() { m_catcher.setMoveState(ClawCatcher::State::Idle); }

    void DollGame::chooseDollPayment() {
        const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
        const int tokens = m_hooks.countItem(kCarnieToken) + m_hooks.countItem(kFunToken);
        if (tokens != 0) {
            m_hooks.choose("How to do it\xA1H", {"Points", "Carnie Token", "Fun Token"}, [this](int choice) { selectDollPayment(choice); });
        } else {
            m_hooks.ask(
                points + "\rThe points for playing Baby Dolls " + std::to_string(kPointsCost) + " Points!\r Sure to continue?",
                false,
                [this] { playWith(1); },
                [] {}
            ); // pay kind 1: points
        }
    }

    // list choice 0/1/2 = Points/Carnie Token/Fun Token
    void DollGame::selectDollPayment(int choice) {
        if (!hasRewardSpace()) {
            m_hooks.message(kFull, kMessageMs);
            return;
        }
        if (choice == 0) { // list choice 0: points
            const std::string points = "IM Points :" + std::to_string(m_hooks.imPoints());
            m_hooks.ask(
                points + "\rThe points for playing Baby Dolls " + std::to_string(kPointsCost) + " Points!\r Sure to continue?",
                true,
                [this] { playWith(1); },
                [] {}
            ); // pay kind 1: points
        } else if (choice == 1 || choice == 2) { // list choices 1 and 2: Carnie Token, Fun Token
            const int payKind = choice + 1; // list choice to pay kind (2 Carnie Tokens, 3 Fun Tokens)
            const std::string rest =
                "Rest of the tokens:" + std::to_string(m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken)); // pay kind 2: Carnie Tokens
            m_hooks.ask(
                rest + "\rThe token for playing Baby Dolls once " + std::to_string(kTokenCost) + "\rContinue\xA1H",
                true,
                [this, payKind] { playWith(payKind); },
                [] {}
            );
        }
    }

    void DollGame::playWith(int payKind) {
        m_confirmed = false;
        if (payKind == 1) { // pay kind 1: points
            if (m_hooks.imPoints() < kPointsCost) {
                m_hooks.message("Please supply more points", kMessageMs);
                return;
            }
        } else if (m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken) < kTokenCost) { // pay kind 2: Carnie Tokens, else Fun Tokens
            m_hooks.message("Not enough carnie tokens", kMessageMs);
            return;
        }
        if (hasRewardSpace()) {
            m_payKind = payKind;
            m_paid = true;
            m_hooks.playMusic(kMusic);
        } else {
            m_hooks.message(kFull, kMessageMs);
        }
        m_confirmed = true;
    }

    void DollGame::askPlayAgain() {
        resetDollsIfAllTaken();
        const int payKind = m_payKind;
        if (payKind == 2 || payKind == 3) { // pay kinds 2 and 3: Carnie Tokens, Fun Tokens
            const std::string rest = "Rest of the tokens:" + std::to_string(m_hooks.countItem(payKind == 2 ? kCarnieToken : kFunToken));
            if (!m_confirmed) {
                m_hooks.ask(rest + "\rPlay again? Need more tokens " + std::to_string(kTokenCost), false, [this, payKind] { playWith(payKind); }, [] {});
            } else {
                playWith(payKind);
            }
            return;
        }
        if (!m_confirmed) {
            m_hooks.ask(
                "IM Points :" + std::to_string(m_hooks.imPoints()) + "\rPlay again? Need more points " + std::to_string(kPointsCost),
                false,
                [this] { playWith(1); },
                [] {}
            ); // pay kind 1: points
        } else {
            playWith(1); // pay kind 1: points
        }
    }

    void DollGame::exit() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void DollGame::quitWithResult() {
        if (m_paid) {
            m_hooks.requestMessage("The result is\xA1G\r" + m_dollText, kExitResultMessageMs);
        }
        exit();
    }

    void DollGame::quitClicked() {
        const bool tokens = m_payKind == 2 || m_payKind == 3; // pay kinds 2 and 3: Carnie Tokens, Fun Tokens
        const std::string exitText = tokens ? "Going to exit\xA1H\r\xA1mDon't need lose token now\xA1n" : "Want to exit the game?\r[Won't lose points now]";
        const std::string quitText = tokens ? "The result would depend on your recent state\rWant to exit\xA1H\r\xA1mNeed to lose token\xA1n"
                                            : "The result would depend on your recent state\rContinue\xA1H\r\xA1mThe recharge points would be decreased\xA1n";
        const ClawCatcher::State state = m_catcher.state();
        if (m_paid && state != ClawCatcher::State::Grasping && state != ClawCatcher::State::Lifting) {
            if (state != ClawCatcher::State::Returning) {
                m_hooks.ask(exitText, false, [this] { exit(); }, [] {});
            } else {
                m_hooks.ask(quitText, false, [this] { quitWithResult(); }, [] {});
            }
        }
        if (!m_paid && m_catcher.isIdle()) {
            exit();
        }
    }

    void DollGame::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip the 2 byte packet header (71/7)
        const int code = reader.u8();
        if (code == 2) { // code 2: play error, not enough points
            m_paid = false;
            m_hooks.requestMessage("Please supply more points", kMessageMs);
            return;
        }
        if (code != 3) { // code 3: reward (i32 item, u8 quantity); others ignored
            return;
        }
        const int itemId = reader.i32();
        const int quantity = reader.u8();
        if (reader.failed()) {
            return;
        }
        const std::string name = m_hooks.itemName(itemId);
        // result 2: doll held in the middle (a real prize), else an honorable prize
        m_dollText = m_catcher.result() == 2 ? "Get rewards\xA1G" + std::to_string(quantity) + " Quantity [ " + name + " ]"
                                             : "Got an honorable prize " + std::to_string(quantity) + " Quantity [ " + name + " ]";
    }

    int DollGame::caughtDollPrize() const {
        if (m_catcher.doll() != -1 && m_catcher.result() == 2) { // result 2: doll held in the middle
            return m_dolls[static_cast<std::size_t>(m_catcher.doll())].kindIdx;
        }
        return 0;
    }

    void DollGame::checkClawCatch() {
        if (m_catcher.state() != ClawCatcher::State::Grasping || m_catcher.grabbed()) {
            return;
        }
        for (int i = kDolls - 1; i >= 0; --i) {
            const int screenY = m_dolls[static_cast<std::size_t>(i)].screen.y;
            const int left = leftEdge(i);
            const int right = rightEdge(i);
            if (m_catcher.result() == 0 || m_catcher.result() == 1) {
                if (left > m_catcher.clawLeftX() && right < m_catcher.clawRightX() && m_catcher.result() != 1 && screenY < kCatchLimitY) {
                    m_catcher.setGrabbed(true);
                    m_catcher.setClosed(true);
                    m_catcher.setResult(1);
                    m_catcher.setDoll(i);
                    m_catcher.setHeldY(screenY);
                }
                if (left <= m_catcher.clawCenterLeftX() && right >= m_catcher.clawCenterRightX() && screenY < kCatchLimitY) {
                    m_catcher.setClosed(true);
                    m_catcher.setResult(2); // result 2: doll held in the middle
                    m_catcher.setGrabbed(true);
                    m_catcher.setDoll(i);
                }
            }
        }
        if (!m_catchRequestSent) {
            m_hooks.send(
                {71, 7, static_cast<std::uint8_t>(m_payKind), static_cast<std::uint8_t>(caughtDollPrize())}
            ); // packet 71/7: u8 pay kind, u8 caught doll kind
            m_catchRequestSent = true;
        }
    }

    void DollGame::setCaughtDollPose() {
        if (m_catcher.grabbed() && m_catcher.catching().frame > kCaughtFrame && m_catcher.doll() != -1) {
            NpcDoll& doll = m_dolls[static_cast<std::size_t>(m_catcher.doll())];
            if (doll.direction == kFacingLeft) {
                doll.direction = kHeldFacingLeft;
            }
            if (doll.direction == kFacingRight) {
                doll.direction = kHeldFacingRight;
            }
        }
    }

    void DollGame::clampHeldDollToClaw() {
        const int doll = m_catcher.doll();
        const int clawY = m_catcher.y() + ClawCatcher::kBasicHeight;
        const int height = spriteSizeOf(doll).height;
        shiftY(doll, kClampRise);
        const int screenY = m_dolls[static_cast<std::size_t>(doll)].screen.y;
        if (screenY < clawY) {
            shiftY(doll, -(clawY - screenY) - height / 3); // a third of the sprite height
        }
    }

    void DollGame::calcHeldDollLiftStep() {
        const int doll = m_catcher.doll();
        const int clawY = m_catcher.y() + ClawCatcher::kBasicHeight;
        const int height = spriteSizeOf(doll).height;
        const int frames = m_catcher.moving().frame + 1;
        const int dollTop = m_dolls[static_cast<std::size_t>(doll)].screen.y - height / 2 - frames * kLiftPerFrame;
        if (dollTop < clawY) {
            const int step = (clawY - dollTop) / frames;
            shiftY(doll, kLiftPerFrame - step);
        }
    }

    void DollGame::finishCatch(const std::string& message) {
        m_hooks.requestMessage(message, kResultMessageMs);
        m_hooks.playSound("sound/wav0152.wav");
        m_catchRequestSent = false;
        m_catcher.clearGrab();
        m_catcher.reset();
        m_paid = false;
        askPlayAgain();
    }

    void DollGame::updateDollCatch() {
        ClawCatcher& claw = m_catcher;
        const ClawCatcher::State state = claw.state();
        if (claw.grabbed() && state == ClawCatcher::State::Grasping && claw.closed() && claw.catching().isLast()) {
            shiftY(claw.doll(), 3); // pixels the doll sags when the claw has closed (3 px)
            claw.setClosed(false);
        }
        if (claw.grabbed() && state == ClawCatcher::State::Lifting && claw.lastLiftFrame() != claw.moving().frame) {
            if (claw.result() == 2 || claw.result() == 1) { // result 2: held in the middle, 1: honorable prize
                calcHeldDollLiftStep();
            }
            claw.setLastLiftFrame(claw.moving().frame);
        }
        if (claw.grabbed() && claw.risen() && (claw.result() == 2 || claw.result() == 1)) { // result 2: held in the middle, 1: honorable prize
            claw.setRisen(false);
            clampHeldDollToClaw();
            claw.setFallsBack(true);
        }
        if (claw.grabbed() && claw.fallsBack() && claw.result() == 1) {
            dropTowardY(claw.doll(), claw.heldY());
        }
        if (claw.grabbed() && claw.x() - ClawCatcher::kStep >= ClawCatcher::kHomeX && claw.state() == ClawCatcher::State::Returning &&
            claw.result() == 2) { // result 2: doll held in the middle
            shiftX(claw.doll(), kCarryStep);
        }
        const bool atHole = claw.x() - ClawCatcher::kStep <= ClawCatcher::kHomeX;
        const bool backOrHome = claw.state() == ClawCatcher::State::Returning || claw.state() == ClawCatcher::State::Idle;
        if (claw.grabbed() && atHole && claw.x() <= ClawCatcher::kHomeX && backOrHome && !claw.droppingPicture().isFirst() &&
            !claw.droppingPicture().isLast() && claw.result() == 2) { // result 2: doll held in the middle
            claw.setDropsIntoHole(true);
        }
        if (claw.dropsIntoHole() && claw.result() == 2) { // result 2: doll held in the middle
            if (m_dolls[static_cast<std::size_t>(claw.doll())].screen.y >= kFloorBottom) {
                claw.setDropsIntoHole(false);
            } else {
                shiftY(claw.doll(), -kDropStep);
            }
        }
        if (claw.grabbed() && atHole && backOrHome && claw.droppingPicture().isLast()) {
            NpcDoll& doll = m_dolls[static_cast<std::size_t>(claw.doll())];
            if (doll.direction == kHeldFacingLeft) {
                doll.direction = kFacingLeft;
            }
            if (doll.direction == kHeldFacingRight) {
                doll.direction = kFacingRight;
            }
            // result 2: doll held in the middle (exchange), else nothing
            finishCatch((claw.result() == 2 ? std::string("To exchange with this doll\r") : std::string("Oh, you get nothing\xA1I\r")) + m_dollText);
            return;
        }
        if (!claw.grabbed() && claw.state() == ClawCatcher::State::Returning && atHole && claw.droppingPicture().isLast()) {
            finishCatch(std::string("Oh, you get nothing\xA1I\r") + m_dollText);
        }
    }

    void DollGame::animateDolls(std::uint64_t nowMs) {
        for (NpcDoll& doll : m_dolls) {
            doll.anim.next(nowMs, m_hooks.frameCount(doll.npc, doll.direction));
        }
    }

    void DollGame::frame(std::uint64_t nowMs) {
        m_catcher.moveX();
        checkClawCatch();
        updateDollCatch();
        setCaughtDollPose();
        animateDolls(nowMs);
        m_catcher.draw();
    }

    void DollGame::update(std::uint64_t nowMs, bool instructionsOpen) {
        constexpr int kMostCatchUp = 10; // most frames run in one update, to avoid a spiral after a stall
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_previousClawX = m_catcher.x();
            for (NpcDoll& doll : m_dolls) {
                doll.previousScreen = doll.screen;
            }
            if (!instructionsOpen) {
                m_formShown = true;
            }
            frame(m_lastFrameMs);
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
