#include "client/subsystems/minigames/DigHoleGame.h"

#include "core/Between.h"
#include "core/ByteReader.h"
#include "protocol/MallMessages.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kCellPitch = 83; // distance between neighbouring hole cells, px
        constexpr int kCellSize = 76; // clickable width and height of one hole cell, px
        constexpr int kCellsLeft = 7 + 143 - 35; // x of the first cell column on screen, px
        constexpr int kCellsTop = 7 - 1 + 20; // y of the first cell row on screen, px
        constexpr int kFieldLeft = 150 - 35; // left edge of the clickable field, px (before the board offset)
        constexpr int kFieldRight = 648 - 35 + 76; // right edge of the clickable field, px (before the board offset)
        constexpr int kFieldTop = 26; // top edge of the clickable field, px
        constexpr int kFieldBottom = 517; // bottom edge of the clickable field, px
        constexpr int kBoardEdge = 800; // board slide distance: the 800 px screen width
        constexpr int kBoardStep = 20; // board slide speed, px per frame
        constexpr int kRiseFast = 5; // prize rise speed in the first phase, px per frame
        constexpr int kRiseSlow = 2; // prize rise speed in the second phase, px per frame
        constexpr int kRiseFastUntil = 30; // prize height where the fast rise ends, px
        constexpr int kRiseUntil = 90; // prize height where the rise ends and the result is shown, px
        const char* const kMusic = "BGM0019"; // background music name

    }

    DigHoleGame::DigHoleGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void DigHoleGame::open(std::uint64_t nowMs) {
        m_lastFrameMs = nowMs;
        m_hooks.send(protocol::imPointsRequest(0)); // request argument 0 (34/1 points request)
    }

    std::string DigHoleGame::pointsText() const { return "IM Points :" + std::to_string(m_imPoints); }

    void DigHoleGame::quitClicked() {
        if (m_revealing) {
            return;
        }
        m_mouseInField = false;
        m_hooks.ask("Want to exit the game?\r[Won't lose points now]", false, [this] { exitGame(); }, [this] { cancelPayment(); });
    }

    void DigHoleGame::exitGame() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void DigHoleGame::cancelPayment() {
        if (!m_hooks.instructionsOpen() && m_paid) {
            m_mouseInField = true;
        }
    }

    void DigHoleGame::digClicked() {
        m_digPressed = true;
        m_digButtonShown = false;
        m_hooks.playSound("sound/wav0154.wav");
    }

    void DigHoleGame::begin() {
        if (!m_formShown) {
            m_formShown = true;
            m_hooks.playMusic(kMusic);
        }
    }

    void DigHoleGame::askToPlay() {
        if (m_hooks.freeBagPlaces() > 0) {
            m_hooks.ask(
                pointsText() + "\rSharpshooter Game requires " + std::to_string(kPlayPoints) + " Points!\r Sure to continue?",
                false,
                [this] { playWithPoints(); },
                [this] { cancelPayment(); }
            );
        } else {
            m_hooks.message("Item bar is full\rNot enough space for the rewards", 1300); // message display time in ms
        }
    }

    void DigHoleGame::playWithPoints() {
        if (m_imPoints < kPlayPoints) {
            m_hooks.message("Please supply more points", 1300); // message display time in ms
        } else if (m_hooks.freeBagPlaces() > 0) {
            m_paid = false;
            m_pointsMode = 1; // pay mode 1: pay with IM Points
            m_paid = true;
        } else {
            m_hooks.message("Item bar is full\rNot enough space for the rewards", 1300); // message display time in ms
        }
    }

    void DigHoleGame::offerReplay() {
        m_hooks.ask(
            pointsText() + "\rPlay again? Need more points " + std::to_string(kPlayPoints),
            false,
            [this] { playWithPoints(); },
            [this] { cancelPayment(); }
        );
    }

    void DigHoleGame::mouseDown(core::Point at) {
        if (m_boardSwapping) {
            return;
        }
        if (!m_hooks.instructionsOpen() && !m_revealing) {
            if (!m_paid) {
                askToPlay();
            } else {
                for (int row = 1; row <= kRows; ++row) {
                    for (int column = 1; column <= kColumns; ++column) {
                        const int left = (column - 1) * kCellPitch + kCellsLeft + m_boardOffsetX;
                        const int top = (row - 1) * kCellPitch + kCellsTop;
                        if (left < at.x && top < at.y && left + kCellSize > at.x && top + kCellSize > at.y) {
                            if (dug(row, column) || m_hooks.requestBoxOpen()) {
                                return;
                            }
                            m_mouseInField = false;
                            m_cellPicked = true;
                            m_pickedColumn = column;
                            m_pickedRow = row;
                        }
                    }
                }
            }
        }
        if (m_paid && !m_revealing && m_pickedColumn != 0 && m_pickedRow != 0) {
            m_hooks.send({71, 20, static_cast<std::uint8_t>(m_pointsMode), 0}); // packet 71/20; last byte FRequestByte is always 0
        }
    }

    void DigHoleGame::mouseUp() {
        if (!m_hooks.instructionsOpen() && !m_revealing && m_paid && !m_hooks.requestBoxOpen()) {
            m_mouseInField = true;
            m_cellPicked = false;
        }
    }

    void DigHoleGame::mouseMove(core::Point at) {
        m_mouse = at;
        if (!m_hooks.instructionsOpen() && m_paid && m_boardOffsetX + kFieldLeft < at.x && m_boardOffsetX + kFieldRight > at.x && at.y > kFieldTop &&
            at.y < kFieldBottom) {
            if (!m_hooks.requestBoxOpen()) {
                m_fingerCursor = true;
                m_mouseInField = true;
                m_cellPicked = false;
            }
        } else {
            m_fingerCursor = false;
            m_mouseInField = false;
        }
    }

    void DigHoleGame::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip the 2 byte packet header (71/20)
        const int arm = reader.u8(); // result code: 1 play error, 2 not enough points, 3 and 4 reward
        if (arm == 2) { // arm 2: play error, not enough points
            m_paid = false;
            m_mouseInField = false;
            m_cellPicked = false;
            m_hooks.requestMessage("Please supply more points", 1300); // message display time in ms
            return;
        }
        // arms 1 (play error), 3 and 4 (reward) continue
        if (arm != 1 && arm != 3 && arm != 4) {
            return;
        }
        m_revealing = true;
        m_paid = false;
        if (m_pickedRow != 0 && m_pickedColumn != 0) {
            m_dug[static_cast<std::size_t>((m_pickedRow - 1) * kColumns + m_pickedColumn)] = true;
        }
        if (arm == 1) { // arm 1: no reward item follows
            return;
        }
        m_itemId = static_cast<int>(reader.u32());
        m_rewardName = m_hooks.itemName(m_itemId);
        m_rewardCount = std::to_string(reader.u8());
    }

    void DigHoleGame::reveal() {
        if (m_revealHeight < kRiseFastUntil) {
            m_revealHeight += kRiseFast;
        } else if (m_revealHeight < kRiseUntil) {
            if (!m_revealSoundPlayed) {
                m_hooks.playSound("sound/Wav1611.wav");
            }
            m_cellPicked = false;
            m_revealSoundPlayed = true;
            m_revealHeight += kRiseSlow;
        } else {
            m_revealHeight = 0;
            m_revealing = false;
            m_revealSoundPlayed = false;
            m_pickedColumn = 0;
            m_pickedRow = 0;
            m_hooks.requestMessage("Obtain " + m_rewardCount + " Quantity " + m_rewardName, 1500); // message display time in ms
            m_hooks.playSound("sound/wav0152.wav");
            offerReplay();
        }
    }

    void DigHoleGame::slideBoard() {
        if (m_boardOffsetX < kBoardEdge && m_boardOffsetX > -kBoardStep) {
            m_digButtonShown = false;
            m_boardOffsetX += kBoardStep;
        } else if (m_boardOffsetX == kBoardEdge) {
            m_boardSwapping = true;
            m_boardOffsetX = -kBoardEdge;
        } else if (m_boardOffsetX > -kBoardEdge - kBoardStep && m_boardOffsetX < 0) {
            m_boardOffsetX += kBoardStep;
            if (m_boardOffsetX == 0) {
                m_dug.fill(false);
                m_boardSwapping = false;
                m_digPressed = false;
                m_digButtonShown = true;
            }
        }
    }

    void DigHoleGame::frame() {
        if (!m_hooks.instructionsOpen()) {
            begin();
        }
        if (m_revealing) {
            reveal();
        }
        if (m_mouseInField || m_cellPicked || m_digPressed) {
            m_digButtonShown = false;
        } else if (m_boardOffsetX == 0) {
            m_digButtonShown = true;
        }
        if (m_digPressed) {
            slideBoard();
            return;
        }
        for (int cell = 1; cell <= kCells; ++cell) {
            if (!m_dug[static_cast<std::size_t>(cell)]) {
                return;
            }
        }
        if (m_paid) {
            slideBoard();
        }
    }

    int DigHoleGame::boardOffset(double amount) const {
        if (std::abs(m_boardOffsetX - m_previousBoardOffsetX) > kBoardStep) {
            return m_boardOffsetX;
        }
        return core::between(m_previousBoardOffsetX, m_boardOffsetX, amount);
    }

    int DigHoleGame::revealHeight(double amount) const {
        if (std::abs(m_revealHeight - m_previousRevealHeight) > kRiseFast) {
            return m_revealHeight;
        }
        return core::between(m_previousRevealHeight, m_revealHeight, amount);
    }

    void DigHoleGame::update(std::uint64_t nowMs) {
        constexpr int kMostCatchUp = 10; // most frames run in one update, to avoid a spiral after a stall
        for (int frames = 0; nowMs >= m_lastFrameMs + kFrameMs && frames < kMostCatchUp; ++frames) {
            m_lastFrameMs += kFrameMs;
            m_previousBoardOffsetX = m_boardOffsetX;
            m_previousRevealHeight = m_revealHeight;
            frame();
        }
        if (nowMs >= m_lastFrameMs + kFrameMs * kMostCatchUp) {
            m_lastFrameMs = nowMs;
        }
    }

}
