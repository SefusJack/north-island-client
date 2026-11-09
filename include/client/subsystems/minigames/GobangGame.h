#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GOBANG_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_GOBANG_GAME_H

#include "client/subsystems/minigames/GobangSeat.h"
#include "client/subsystems/minigames/GobangTable.h"
#include "core/Bytes.h"
#include "core/Point.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string>

namespace client {

    /// @brief Five in a Row, the game's side (original LD_MiniGame.pas TGobang with the packets of RecvGobangPacket, 57/11): tables, seats, board, turn, game end and move countdown.
    class GobangGame {
        public:
            static constexpr int kTables = 16; // tables in the hall
            static constexpr int kSize = 15; // board is 15 x 15 cells
            // screen position of the board's first cell
            static constexpr core::Point kBoardOrigin{134, 121}; // x, y in px
            static constexpr int kCell = 24; // cell size, px
            static constexpr std::uint64_t kBlinkMs = 3000; // milliseconds after the end packet before the winning line blinks
            static constexpr std::uint64_t kBackToHallMs = 5000; // milliseconds after the end packet before going back to the hall

            /// @brief Callbacks into the client for the server, messages, sounds and player data.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, int)> notice;
                std::function<void(const std::string&, int)> message;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
                std::function<void(const std::string&)> sound;
                std::function<std::int32_t()> ownId;
                std::function<std::int64_t()> gold;
                std::function<GobangSeat()> ownSeat;
                std::function<std::optional<GobangSeat>(std::int32_t)> playerOf;
                std::function<void(int)> anteError;
                std::function<void()> backToHall;
            };

            /// @brief Creates the game with its callbacks.
            /// @param hooks Callbacks into the client.
            explicit GobangGame(
                Hooks hooks
            )
                : m_hooks(std::move(hooks)) {}

            /// @brief Clears the board and game state (original ResetGobangGame).
            void reset();

            /// @brief Handles a 57/11 server packet by its sub id (original RecvGobangPacket).
            /// @param payload The whole packet payload.
            /// @param nowMs Current time in ms.
            void receive(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Left click on the board: a free cell asks the server for the stone with 57/11/5 (original GobangMouseDown).
            /// @param mouse Screen point.
            void click(core::Point mouse);

            /// @brief Per-frame logic of DrawGobangBoard: move countdown, opponent leaving, game end and winning line blinking.
            /// @param nowMs Current time in ms.
            void advance(std::uint64_t nowMs);

            /// @brief Answer Confirm to "Want to start the game?": checks the ante and asks the game to begin with 57/11/4 (original TGobang.OnGobangOpponentLeft).
            void startOrLeave();

            /// @brief A table of the hall.
            /// @param number Table number 1..16 (0 is the empty one).
            /// @return Const reference to the table.
            const GobangTable& table(int number) const { return m_tables.at(static_cast<std::size_t>(number)); }

            /// @brief A table of the hall.
            /// @param number Table number 1..16 (0 is the empty one).
            /// @return Mutable reference to the table.
            GobangTable& table(int number) { return m_tables.at(static_cast<std::size_t>(number)); }

            /// @brief First seat of a table.
            /// @param number Table number.
            /// @return The seat.
            const GobangSeat& seat1(int number) const { return m_seat1.at(static_cast<std::size_t>(number)); }

            /// @brief Second seat of a table.
            /// @param number Table number.
            /// @return The seat.
            const GobangSeat& seat2(int number) const { return m_seat2.at(static_cast<std::size_t>(number)); }

            /// @brief Content of a board cell.
            /// @param column Column 1..15.
            /// @param row Row 1..15.
            /// @return 0 nothing, 1 and 2 a stone, 3 and 4 a stone of the winning line in its off beat.
            int cell(int column, int row) const { return m_board.at(static_cast<std::size_t>(column)).at(static_cast<std::size_t>(row)); }

            /// @brief The table the player sits at (original GobangForm.FTableIndex).
            /// @return Table number, 0 for none.
            int tableIndex() const { return m_tableIndex; }

            /// @brief Sets the table the player sits at.
            /// @param number Table number, 0 for none.
            void setTableIndex(int number) { m_tableIndex = number; }

            /// @brief The player's table (the empty table 0 while he sits nowhere).
            /// @return The table.
            const GobangTable& mine() const { return table(m_tableIndex); }

            /// @brief Whether a game has started at the player's table.
            /// @return True when started.
            bool started() const { return m_started; }

            /// @brief Whether the game is over.
            /// @return True when over.
            bool over() const { return m_over; }

            /// @brief Result text of the finished game.
            /// @return The text.
            const std::string& resultText() const { return m_resultText; }

            /// @brief Whether a player is in the hall (original FPlayerIds).
            /// @param id Player id.
            /// @return True when in the hall.
            bool inHall(std::int32_t id) const { return m_inHall.count(id) != 0; }

            /// @brief The own player's side (original FMyHuman).
            /// @return The seat.
            const GobangSeat& me() const { return m_me; }

            /// @brief The opponent's side (original FOpponentHuman).
            /// @return The seat.
            const GobangSeat& opponent() const { return m_opponent; }

            /// @brief Position of the last stone put (0 / 0: none yet).
            /// @return Screen position.
            core::Point lastStone() const { return m_lastStone; }

            /// @brief When the last stone was put.
            /// @return Time in ms.
            std::uint64_t lastStoneMs() const { return m_lastStoneMs; }

            /// @brief When the packet that ended the game came.
            /// @return Time in ms.
            std::uint64_t packetMs() const { return m_packetMs; }

            /// @brief The seconds bars to draw for the move to make (original DrawGobangCountdown's Left2).
            /// @return Number of bars.
            int secondsBars() const { return m_bars; }

            /// @brief The board cell under a screen point.
            /// @param mouse Screen point.
            /// @return Column and row counted from 1; column 0 outside the board.
            static core::Point cellAt(core::Point mouse);

            /// @brief The ante in gold a table shows (original RecvTableAnte).
            /// @param number Table number.
            /// @return Gold.
            int anteShown(int number) const { return m_anteShown.at(static_cast<std::size_t>(number)); }

            /// @brief The ante in gold for an ante code.
            /// @param code Ante code 0..4.
            /// @return Gold, 0 for an unknown code.
            static int anteGold(int code);

            /// @brief Sets the board's two players from the table the player sits at (original SetupGobangPlayers).
            void setupPlayers();

            /// @brief Whether the own turn is up (the opponent seated, nothing pending).
            /// @return True when it is the own move.
            bool myMove() const { return !mine().movePending && m_opponent.id != 0; }

        private:
            /// @brief Handles 57/11/3: a player took a seat.
            /// @param number Table number.
            /// @param seat Seat 1 or 2.
            /// @param id Player id.
            void seatTaken(int number, int seat, std::int32_t id);

            /// @brief Handles 57/11/4: a seat was freed.
            /// @param number Table number.
            /// @param seat Seat 1 or 2.
            void seatFreed(int number, int seat);

            /// @brief Handles 57/11/6: a stone, the end of a game, or back to the hall.
            /// @param payload The whole packet payload.
            /// @param nowMs Current time in ms.
            void stonePacket(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Turns the winning line's stones to their off beat and back (original BlinkWinningLine).
            /// @param mode 0 acts only when the line shows, 1 only when it is hidden.
            void blinkWinningLine(int mode);

            /// @brief The move countdown: 20 seconds, a bar each 2, with a tick as the bars go down (original DrawGobangCountdown).
            /// @param nowMs Current time in ms.
            void countdown(std::uint64_t nowMs);

            Hooks m_hooks;
            std::array<GobangTable, kTables + 1> m_tables{};
            std::array<GobangSeat, kTables + 1> m_seat1{};
            std::array<GobangSeat, kTables + 1> m_seat2{};
            std::array<std::array<int, kSize + 1>, kSize + 1> m_board{};
            std::array<int, kTables + 1> m_anteShown{};
            std::set<std::int32_t> m_inHall;
            GobangSeat m_me;
            GobangSeat m_opponent;
            int m_tableIndex = 0;
            int m_clickedCell = 0;
            bool m_started = false;
            bool m_over = false;
            bool m_overSound = false;
            bool m_endShown = false;
            std::string m_resultText;
            std::array<core::Point, 6> m_winLine{}; // 5 winning cells counted from 1, so 6 slots
            std::uint64_t m_packetMs = 0;
            std::uint64_t m_moveStartMs = 0;
            std::uint64_t m_lastStoneMs = 0;
            core::Point m_lastStone;
            int m_turnCountdown = 0;
            int m_bars = 0;
    };

}

#endif
