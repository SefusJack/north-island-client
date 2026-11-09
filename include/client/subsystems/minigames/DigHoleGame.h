#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_DIG_HOLE_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_DIG_HOLE_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"
#include "core/Point.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The dig hole game (original LD_DigHoleGame.pas TDigHole, sport kind 20): a board of holes, paid with points, with prizes rising out.
    class DigHoleGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kColumns = 7; // columns of holes on the board
            static constexpr int kRows = 6; // rows of holes on the board
            static constexpr int kCells = kColumns * kRows;
            static constexpr int kPlayPoints = 15; // IM Points one play costs

            /// @brief Callbacks into the client for the request box and instructions state.
            struct Hooks : GameHooks {
                std::function<bool()> requestBoxOpen;
                std::function<bool()> instructionsOpen;
            };

            /// @brief Creates the game with its callbacks.
            /// @param hooks Callbacks into the client.
            explicit DigHoleGame(
                Hooks hooks
            );

            /// @brief Starts the game and asks for the account's points (original TDigHole.Create, request 34/1).
            /// @param nowMs Current time in ms, the first frame's reference.
            void open(std::uint64_t nowMs);

            /// @brief Stores the account's points when 35/4 arrives while the sport runs (original FImPoints).
            /// @param points Points.
            void pointsArrived(std::int32_t points) { m_imPoints = points; }

            /// @brief The Quit button: asks to exit unless a prize is rising (original TDigHoleForm.QuitDigHoleClick).
            void quitClicked();

            /// @brief Stops the music and ends the game (original ExitDigHoleGame, also the instructions' Leave).
            void exitGame();

            /// @brief The Dig button: the board is to slide away (original DigButtonClick).
            void digClicked();

            /// @brief Shows the form and plays the music (original DigHoleForm.ShowCtrl, the instructions' Begin).
            void begin();

            /// @brief Left button down: asks to play when unpaid, else picks the hole under the mouse (original DigHoleMouseDown).
            /// @param at Mouse position.
            void mouseDown(core::Point at);

            /// @brief Mouse move: shows the finger cursor over the board once paid (original DigHoleMouseMove).
            /// @param at Mouse position.
            void mouseMove(core::Point at);

            /// @brief Left button up (original DigHoleMouseUp).
            void mouseUp();

            /// @brief Handles server packet 71/20: arms 1 and 2 are play errors (RecvDigHolePlayError), 3 and 4 rewards (RecvDigHoleReward).
            /// @param payload Packet bytes.
            void receive(const core::Bytes& payload);

            /// @brief Runs the frames (30 ms each) that have passed (original UpdateDigHoleGame).
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

            /// @brief Whether the form is shown (original DigHoleForm.FIsVisible).
            /// @return True when shown.
            bool formShown() const { return m_formShown; }

            /// @brief Whether the Dig button shows (original FDigBtn).
            /// @return True when shown.
            bool digButtonShown() const { return m_digButtonShown; }

            /// @brief Whether the last mouse move chose the finger cursor (cursor type 99) instead of the arrow.
            /// @return True for the finger cursor.
            bool fingerCursor() const { return m_fingerCursor; }

            /// @brief Board slide offset of the current frame.
            /// @return X offset in pixels.
            int boardOffset() const { return m_boardOffsetX; }

            /// @brief How far the time has come from the last frame towards the next.
            /// @param nowMs Current time in ms.
            /// @return Fraction from 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Board offset drawn that part of the way from the previous frame (a jump is drawn at once).
            /// @param amount Fraction from 0 to 1.
            /// @return X offset in pixels.
            int boardOffset(double amount) const;

            /// @brief Prize rise height drawn that part of the way from the previous frame (a new rise is drawn at once).
            /// @param amount Fraction from 0 to 1.
            /// @return Height in pixels.
            int revealHeight(double amount) const;

            /// @brief Whether the new board is sliding in after the old one went off.
            /// @return True while swapping.
            bool boardSwapping() const { return m_boardSwapping; }

            /// @brief Whether a hole is dug.
            /// @param row Row, 1-based.
            /// @param column Column, 1-based.
            /// @return True when dug.
            bool dug(int row, int column) const { return m_dug[static_cast<std::size_t>((row - 1) * kColumns + column)]; }

            /// @brief Row of the hole the prize rises out of.
            /// @return Row, 1-based, 0 for none.
            int risingRow() const { return m_revealSoundPlayed ? m_pickedRow : 0; }

            /// @brief Column of the hole the prize rises out of.
            /// @return Column, 1-based, 0 for none.
            int risingColumn() const { return m_revealSoundPlayed ? m_pickedColumn : 0; }

            /// @brief Item id of the prize.
            /// @return Item id.
            int itemId() const { return m_itemId; }

            /// @brief How far the prize has risen.
            /// @return Height in pixels.
            int revealHeight() const { return m_revealHeight; }

            /// @brief Whether the finger shows over the board.
            /// @return True when shown.
            bool fingerShown() const { return m_mouseInField; }

            /// @brief Whether the finger is drawn pressed (a hole is picked).
            /// @return True when pressed.
            bool fingerPressed() const { return m_cellPicked; }

            /// @brief Last mouse position.
            /// @return The position.
            core::Point mouse() const { return m_mouse; }

            /// @brief The account's points (original FImPoints).
            /// @return Points, -1 until known.
            int imPoints() const { return m_imPoints; }

            /// @brief Whether a play is paid (original FPaid).
            /// @return True when paid.
            bool paid() const { return m_paid; }

            /// @brief Whether a prize is being revealed (original FRevealing).
            /// @return True while revealing.
            bool revealing() const { return m_revealing; }

        private:
            /// @brief Runs one 30 ms frame (original UpdateDigHoleGame).
            void frame();

            /// @brief One step of the board sliding off to the right and a new one in from the left (original SlideDigHoleBoard).
            void slideBoard();

            /// @brief Resumes play after the payment was declined (original TDigHole.CancelDigHolePayment).
            void cancelPayment();

            /// @brief Asks to pay the points to play, if the bag has room (original AskPlayDigHole).
            void askToPlay();

            /// @brief Takes the points and marks the play paid (original TDigHole.PlayDigHoleWithPoints).
            void playWithPoints();

            /// @brief Offers to play again (original OfferDigHoleReplay).
            void offerReplay();

            /// @brief One step of the prize rising out of the hole; at the end shows the reward and offers a replay.
            void reveal();

            /// @brief Text of the points line.
            /// @return The text.
            std::string pointsText() const;

            Hooks m_hooks;
            std::uint64_t m_lastFrameMs = 0;
            std::array<bool, kCells + 1> m_dug{};
            bool m_formShown = false;
            bool m_digButtonShown = true;
            bool m_fingerCursor = false;
            bool m_paid = false;
            bool m_revealing = false;
            bool m_revealSoundPlayed = false;
            int m_imPoints = -1;
            int m_revealHeight = 0;
            int m_previousRevealHeight = 0;
            bool m_boardSwapping = false;
            int m_boardOffsetX = 0;
            int m_previousBoardOffsetX = 0;
            int m_pointsMode = 0;
            int m_itemId = 0;
            std::string m_rewardName;
            std::string m_rewardCount;
            int m_pickedColumn = 0;
            int m_pickedRow = 0;
            bool m_mouseInField = false;
            bool m_cellPicked = false;
            bool m_digPressed = false;
            core::Point m_mouse;
    };

}

#endif
