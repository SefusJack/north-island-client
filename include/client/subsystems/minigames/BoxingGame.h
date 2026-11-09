#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_BOXING_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_BOXING_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "core/Rect.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <string>

namespace client {

    /// @brief The Golden Glove boxing minigame: a swinging power gauge, three punches added to a score out of 800, the server gives the prize.
    /// @details Original: MZ_BoxingGame.pas TSportBoxing; game kind 9, packet 71/9. A knock-out is more than 800. Everything runs a frame (30 ms) at a time, as the game's update and draw do.
    class BoxingGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kGaugeWidth = 284; // full width of the power gauge, px
            static constexpr int kPunches = 3; // punches per play
            static constexpr int kTotalScore = 800; // score cap of the three punches added (a knock-out is more than this)
            static constexpr core::Rect kPunchButton{242, 312, 339, 201}; // clickable boxer area on the form: x, y, width, height in px

            /// @brief What the frame shows of the fight: the monster's frame, its shake, the boxer's frame and whether the boxer is lit.
            /// @details Original: DrawBoxingPunch.
            struct Pose {
                int monsterFrame = 0;
                int shake = 0;
                int boxerFrame = 0;
                bool lit = false;
            };

            /// @brief The score board: the three punches (-1 for one not made yet) and what is left of 800.
            /// @details Original: TBoxingGrade; a left of 0 is the game's nil.
            struct Scores {
                int punch[kPunches] = {-1, -1, -1};
                int left = kTotalScore;
            };

            /// @brief Creates the game with its callbacks into the client.
            /// @param hooks Callbacks for sending, messages, sounds and bag queries.
            explicit BoxingGame(
                GameHooks hooks
            );

            /// @brief Starts the game: resets the state and asks for the points.
            /// @details Original: TSportBoxing.Create; the points are asked for (packet 34/1).
            /// @param nowMs Current time in milliseconds.
            void open(std::uint64_t nowMs);

            /// @brief Tells whether the mouse is over the boxer's button.
            /// @details Original: BoxingMouseMove.
            /// @param overPunchButton True when the mouse is over the button.
            void moved(bool overPunchButton);

            /// @brief Handles a mouse press: asks to pay or asks for a punch.
            /// @details Original: BoxingMouseDown.
            /// @param instructionsOpen True while the instructions are shown.
            void pressed(bool instructionsOpen);

            /// @brief Handles the boxing form's Quit button.
            /// @details Original: QuitBoxingClick.
            void quitClicked();

            /// @brief Runs the frames that have passed.
            /// @details Original: UpdateBoxingGame and DrawBoxingGame.
            /// @param nowMs Current time in milliseconds.
            /// @param instructionsOpen True while the instructions are shown.
            void update(std::uint64_t nowMs, bool instructionsOpen);

            /// @brief Handles the server's boxing packet.
            /// @details Original: RecvBoxingPlayResult and RecvBoxingReward, packet 71/9: 1 and 2 the answer to the play request, 3 and 4 the reward.
            /// @param payload Packet payload.
            void receive(const core::Bytes& payload);

            /// @brief Whether the boxing form is shown.
            /// @return True when shown.
            bool formShown() const { return m_formShown; }

            /// @brief The width of the power gauge now.
            /// @return Width in pixels.
            int powerWidth() const { return m_powerWidth; }

            /// @brief How far the time has come from the last frame towards the next, for drawing between frames with free frames.
            /// @param nowMs Current time in milliseconds.
            /// @return A fraction from 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief The gauge width before the last frame.
            /// @return Width in pixels.
            int previousPowerWidth() const { return m_previousPowerWidth; }

            /// @brief The prize figure's vertical position before the last frame.
            /// @return Position in pixels.
            int previousRewardY() const { return m_previousRewardY; }

            /// @brief What the last frame shows of the fight.
            /// @return The pose.
            const Pose& pose() const { return m_pose; }

            /// @brief The score board.
            /// @return The scores.
            const Scores& scores() const { return m_scores; }

            /// @brief Which figure slides in (0 lost, 1 knock-out).
            /// @details Original: DrawBoxingReward.
            /// @return 0 lost, 1 knock-out, -1 none yet.
            int rewardFigure() const { return m_rewardShown ? (m_scores.left != 0 ? 0 : 1) : -1; }

            /// @brief How far down the prize figure is.
            /// @return Vertical position in pixels.
            int rewardY() const { return m_rewardY; }

        private:
            /// @brief Runs one 30 ms frame: gauge, punch, score limit, reward.
            /// @details Original: UpdateBoxingGame and DrawBoxingGame.
            /// @param instructionsOpen True while the instructions are shown.
            void frame(bool instructionsOpen);

            /// @brief Swings the power gauge until the punch is asked for, then fixes the strength.
            /// @details Original: UpdateBoxingGauge.
            void updateGauge();

            /// @brief Plays out the punch: the monster and boxer frames, shake, sound, score.
            /// @details Original: DrawBoxingPunch.
            void drawPunch();

            /// @brief Slides the prize figure, sends the result, shows the message and offers the replay.
            /// @details Original: DrawBoxingReward.
            void drawReward();

            /// @brief Adds the punches up on the score board.
            void scoreBoard();

            /// @brief Asks how to pay for a game.
            /// @details Original: ChooseBoxingPayment.
            void askToPay();

            /// @brief Handles the payment way chosen from the list and asks again in the main box.
            /// @details Original: SelectBoxingPayment.
            /// @param choice List choice: 0 points, 1 Carnie Token, 2 Fun Token.
            void chooseWay(int choice);

            /// @brief Pays and starts a game if the means and bag space are there.
            /// @details Original: PlayWithPoints, PlayWithCarnieToken, PlayWithFunToken.
            /// @param payKind 1 points, 2 Carnie Tokens, 3 Fun Tokens.
            void playWith(int payKind);

            /// @brief Starts a new fight and resets the score board.
            /// @details Original: StartBoxingGame, ResetBoxingScoreBoard.
            void startGame();

            /// @brief Offers a replay with the same payment.
            /// @details Original: OfferBoxingReplay.
            void offerReplay();

            /// @brief Stops the music and ends the game.
            /// @details Original: ExitBoxing.
            void exit();

            /// @brief Sends the play request with the answer 2 (quit).
            /// @details Original: SendQuitBoxing.
            void sendQuit();

            /// @brief Whether the bag has a free place for the rewards.
            /// @return True when at least one place is free.
            bool hasRewardSpace() const { return m_hooks.freeBagPlaces() > 0; }

            /// @brief A random number from 0 to count - 1.
            /// @param count Exclusive upper bound.
            /// @return The random number.
            int random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

            GameHooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            std::uint64_t m_lastFrameMs = 0;
            bool m_formShown = false;
            bool m_paid = false;
            int m_payKind = 1; // payment kind sent with the play request (starts as 1)
            bool m_gaugeRising = true;
            int m_powerWidth = 0;
            int m_powerLevel = 0;
            bool m_punchRequested = false;
            bool m_punchBlocked = false;
            bool m_rewardShown = false;
            bool m_overPunch = false;
            int m_punchCount = 0;
            int m_punchScore = 0;
            int m_holdFrames = 0;
            int m_boxerFrame = 0;
            int m_boxerTick = 60; // frames since the boxer's last advance; starts above the advance ticks
            int m_rewardStep = 0;
            int m_rewardY = 30; // prize figure height, px (start value)
            int m_previousPowerWidth = 0;
            int m_previousRewardY = 30; // prize figure height of the previous frame, px
            int m_answer = 0;
            bool m_quitting = false;
            std::string m_rewardText;
            Pose m_pose;
            Scores m_scores;
    };

}

#endif
