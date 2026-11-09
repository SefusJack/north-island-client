#include "client/subsystems/minigames/GobangGame.h"

#include "core/ByteReader.h"

#include <cmath>

namespace client {

    namespace {

        constexpr int kMoveSeconds = 20; // time allowed per move, seconds
        constexpr int kSecondsPerBar = 2; // seconds one bar of the countdown meter stands for
        constexpr int kWinCells = 5; // stones in a row needed to win
        constexpr int kMostGold = 9999999; // most gold a player may hold, gold
        constexpr int kAnteGold[] = {0, 1000, 5000, 10000, 100000}; // ante in gold per ante code (code 0 = no ante)
        constexpr int kAnteCodes = 5; // number of ante codes (entries of kAnteGold)
        constexpr int kNoticeMs = 1200; // notice display time, ms
        constexpr std::uint64_t kBlinkStepMs = 500; // time per blink of the winning line, ms

    }

    int GobangGame::anteGold(int code) { return code >= 0 && code < kAnteCodes ? kAnteGold[code] : 0; }

    core::Point GobangGame::cellAt(core::Point mouse) {
        if (mouse.x < kBoardOrigin.x || mouse.y < kBoardOrigin.y) {
            return {0, 0};
        }
        return {(mouse.x - kBoardOrigin.x) / kCell + 1, (mouse.y - kBoardOrigin.y) / kCell + 1};
    }

    void GobangGame::reset() {
        m_board = {};
        m_moveStartMs = 0;
        m_lastStoneMs = 0;
        m_turnCountdown = 0;
        m_clickedCell = 0;
        m_started = false;
        m_over = false;
        m_overSound = false;
        m_resultText.clear();
        m_winLine = {};
    }

    void GobangGame::click(core::Point mouse) {
        constexpr int kRight = 494; // right edge of the clickable board, px
        constexpr int kBottom = 481; // bottom edge of the clickable board, px
        if (!m_started || mine().movePending || mouse.x < kBoardOrigin.x || mouse.y < kBoardOrigin.y || mouse.x > kRight || mouse.y > kBottom) {
            return;
        }
        const core::Point at = cellAt(mouse);
        if (at.x > kSize || at.y > kSize) {
            return;
        }
        m_clickedCell = (at.x - 1) * kSize + at.y - 1;
        if (cell(at.x, at.y) == 0) {
            table(m_tableIndex).movePending = true;
            m_hooks.send({57, 11, 5, static_cast<std::uint8_t>(m_clickedCell)}); // packet 57/11/5: u8 cell number
        }
    }

    void GobangGame::setupPlayers() {
        m_me = m_hooks.ownSeat();
        m_me.look.equipmentLooks[3] = 0; // equipment slot 3 (weapon) cleared: no weapon at the hall
        const GobangSeat& first = m_seat1.at(static_cast<std::size_t>(m_tableIndex));
        const GobangSeat& second = m_seat2.at(static_cast<std::size_t>(m_tableIndex));
        if (first.id != m_hooks.ownId()) {
            m_opponent = first;
        } else if (second.id != m_hooks.ownId()) {
            m_opponent = second;
        }
    }

    void GobangGame::startOrLeave() {
        const GobangTable& at = mine();
        const std::int64_t gold = m_hooks.gold();
        if (gold < at.ante) {
            m_hooks.anteError(1); // ante error 1: not enough gold
        } else if (gold + at.ante > kMostGold) {
            m_hooks.anteError(2); // ante error 2: too much gold
        } else if ((at.player1 == m_hooks.ownId() && at.player2 == 0) || (at.player2 == m_hooks.ownId() && at.player1 == 0)) {
            m_hooks.notice("Opponent left", kNoticeMs);
        } else {
            m_hooks.send({57, 11, 4}); // packet 57/11/4: ask the game to begin
        }
    }

    void GobangGame::seatTaken(int number, int seat, std::int32_t id) {
        GobangTable& at = m_tables.at(static_cast<std::size_t>(number));
        at.occupied = false;
        GobangSeat& place = seat == 1 ? m_seat1.at(static_cast<std::size_t>(number)) : m_seat2.at(static_cast<std::size_t>(number));
        (seat == 1 ? at.player1 : at.player2) = id;
        if (id != m_hooks.ownId()) {
            const auto found = m_inHall.count(id) != 0 ? m_hooks.playerOf(id) : std::nullopt;
            place = found.value_or(GobangSeat{});
        } else {
            place = m_hooks.ownSeat();
            at.movePending = seat == 2; // seat 2 moves second: waits for the opponent
            at.turn = seat;
            m_tableIndex = number;
        }
        place.look.equipmentLooks[3] = 0; // equipment slot 3 (weapon) cleared: no weapon at the hall
        const std::int32_t other = seat == 1 ? at.player2 : at.player1;
        if (other == m_hooks.ownId()) {
            m_hooks.ask("Want to start the game?", [this] { startOrLeave(); }, m_hooks.backToHall);
        }
    }

    void GobangGame::seatFreed(int number, int seat) {
        GobangTable& at = m_tables.at(static_cast<std::size_t>(number));
        at.occupied = false;
        GobangSeat& place = seat == 1 ? m_seat1.at(static_cast<std::size_t>(number)) : m_seat2.at(static_cast<std::size_t>(number));
        if (place.id == m_hooks.ownId()) {
            m_tableIndex = 0;
        }
        place.id = 0;
        (seat == 1 ? at.player1 : at.player2) = 0;
    }

    void GobangGame::stonePacket(const core::Bytes& payload, std::uint64_t nowMs) {
        // payload bytes: [3] kind, [4] side, [5] cell (kind 1) or [5..9] winning cells (kind 2); at least 5 bytes
        if (payload.size() < 5) {
            return;
        }
        const int kind = payload[3];
        const int side = payload[4];
        if (kind == 1 && payload.size() > 5) { // kind 1: a stone was put (u8 cell at [5])
            const int at = payload[5];
            m_board.at(static_cast<std::size_t>(at / kSize + 1)).at(static_cast<std::size_t>(at % kSize + 1)) = side;
            m_hooks.sound("Sound\\wav0154.wav");
            m_lastStone = {(at / kSize) * kCell + kBoardOrigin.x, (at % kSize) * kCell + kBoardOrigin.y};
            if (mine().turn != side) {
                table(m_tableIndex).movePending = false;
                m_turnCountdown = 2; // first countdown tick at 2 bars left
            } else {
                table(m_tableIndex).movePending = true;
            }
            m_lastStoneMs = nowMs;
            m_moveStartMs = nowMs;
        } else if (kind == 2) { // kind 2: the game ended (side 0 draw, else the winner; 5 winning cells follow)
            for (int index = 1; index <= kWinCells && payload.size() > static_cast<std::size_t>(index + 4); ++index) { // winning cells start at byte 5
                const int at = payload[static_cast<std::size_t>(index + 4)];
                m_winLine.at(static_cast<std::size_t>(index)) = {at / kSize + 1, at % kSize + 1};
            }
            m_packetMs = nowMs;
            m_over = true;
            m_overSound = true; // play the win or lose sound once
            m_resultText = side == 0 ? "Draw Game" : mine().turn == side ? "You Win " : "You Lose";
        } else if (kind == 3) { // kind 3: back to the hall
            m_hooks.backToHall();
        }
    }

    void GobangGame::receive(const core::Bytes& payload, std::uint64_t nowMs) {
        if (payload.size() < 3) { // 3 byte header: main 57, sub 11, arm
            return;
        }
        core::ByteReader reader(payload, 3); // skip the 3 byte header
        switch (payload[2]) {
            case 1: { // 57/11/1: hall id list (u16 count, i32 ids)
                const int count = reader.u16();
                for (int index = 0; index < count && !reader.failed(); ++index) {
                    m_inHall.insert(reader.i32());
                }
                break;
            }
            case 2: // 57/11/2: table ante (u8 table, u8 ante code)
            case 7: { // 57/11/7: table ante, same layout as 57/11/2
                const int number = reader.u8();
                const int code = reader.u8();
                if (!reader.failed() && number >= 1 && number <= kTables && code < kAnteCodes) {
                    m_anteShown.at(static_cast<std::size_t>(number)) = kAnteGold[code];
                }
                break;
            }
            case 3: { // 57/11/3: a seat taken (u8 table, u8 seat, i32 player id)
                const int number = reader.u8();
                const int seat = reader.u8();
                const std::int32_t id = reader.i32();
                if (!reader.failed() && number >= 1 && number <= kTables && (seat == 1 || seat == 2)) {
                    seatTaken(number, seat, id);
                }
                break;
            }
            case 4: { // 57/11/4: a seat freed (u8 table, u8 seat)
                const int number = reader.u8();
                const int seat = reader.u8();
                if (!reader.failed() && number >= 1 && number <= kTables && (seat == 1 || seat == 2)) {
                    seatFreed(number, seat);
                }
                break;
            }
            case 5: { // 57/11/5: a game begins at a table (u8 table)
                const int number = reader.u8();
                if (reader.failed() || number < 1 || number > kTables) {
                    return;
                }
                m_tables.at(static_cast<std::size_t>(number)).occupied = true;
                m_endShown = false;
                if (m_tableIndex != number) {
                    return;
                }
                setupPlayers();
                m_started = true;
                m_hooks.notice("The game started", kNoticeMs);
                m_moveStartMs = nowMs;
                break;
            }
            case 6: stonePacket(payload, nowMs); break; // 57/11/6: a stone, the end of a game, or back to the hall
            default: break; // unknown arm: ignored
        }
    }

    void GobangGame::blinkWinningLine(int mode) {
        const core::Point first = m_winLine.at(1);
        if (first.x < 1 || first.y < 1) {
            return;
        }
        const int shown = m_board.at(static_cast<std::size_t>(first.x)).at(static_cast<std::size_t>(first.y));
        const bool isStone = shown == 1 || shown == 2; // cell values 1 and 2 are the two sides' stones
        if ((mode == 0 && !isStone) || (mode == 1 && isStone)) {
            return;
        }
        for (int index = 1; index <= kWinCells; ++index) {
            int& value = m_board.at(static_cast<std::size_t>(m_winLine.at(static_cast<std::size_t>(index)).x))
                             .at(static_cast<std::size_t>(m_winLine.at(static_cast<std::size_t>(index)).y));
            value = 5 - value; // swaps 1<->4 and 2<->3 (stone <-> off beat)
        }
    }

    void GobangGame::countdown(std::uint64_t nowMs) {
        m_bars = 0;
        if (m_moveStartMs == 0) {
            return;
        }
        const double elapsed = static_cast<double>(nowMs - m_moveStartMs) / 1000.0; // ms per second
        const int left = static_cast<int>(std::ceil(kMoveSeconds - elapsed)) / kSecondsPerBar;
        if (left == 0) {
            m_moveStartMs = 0;
        }
        if (!mine().movePending && left == m_turnCountdown) {
            m_hooks.sound("sound\\wav0145.wav");
            if (m_turnCountdown == 0) {
                m_turnCountdown = 3; // wraps back to 2 after the decrement below
            }
            --m_turnCountdown;
        }
        m_bars = left > 0 ? left : 0;
    }

    void GobangGame::advance(std::uint64_t nowMs) {
        if (!m_started) {
            return;
        }
        m_bars = 0;
        const int turn = mine().turn;
        if (turn != 1 && turn != 2 && !m_endShown && !m_over) { // turn 1 or 2 is the own side; neither means the table was left
            m_hooks.message("Opponent left!", kNoticeMs);
            m_endShown = true;
            m_hooks.backToHall();
            return;
        }
        if (m_over) {
            const std::uint64_t since = nowMs - m_packetMs;
            if (since > kBackToHallMs) {
                if (!m_endShown) {
                    m_endShown = true;
                    m_hooks.backToHall();
                }
            } else if (since > kBlinkMs) {
                m_endShown = false;
                blinkWinningLine(1);
                if (m_overSound) {
                    if (m_resultText == "You Win ") {
                        m_hooks.sound("sound\\wav0153.wav");
                    }
                    if (m_resultText == "You Lose") {
                        m_hooks.sound("sound\\wav1607.wav");
                    }
                    m_overSound = false;
                }
            } else {
                for (int step = 5; step >= 1; --step) { // up to 5 quick blinks of 500 ms before the last showing
                    if (since > kBlinkStepMs * static_cast<std::uint64_t>(step)) {
                        blinkWinningLine(step % 2 == 1 ? 0 : 1);
                        break;
                    }
                }
            }
            return;
        }
        if (myMove()) {
            countdown(nowMs);
        }
    }

}
