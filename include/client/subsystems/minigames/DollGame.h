#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_DOLL_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_DOLL_GAME_H

#include "client/subsystems/minigames/ClawCatcher.h"
#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/NpcDoll.h"
#include "core/Bytes.h"
#include "core/Size.h"
#include "formats/NpcInfo.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>
#include <string>

namespace client {

    /// @brief The claw machine "Baby Dolls" (original MZ_MiniGame.pas TSportCatchDoll with MZ_Catcher.pas and MZ_Doll.pas; game kind 7, 71/7).
    class DollGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kDolls = 10; // most dolls in the machine at once, 1-based

            /// @brief Callbacks into the client for NPC and sprite data.
            struct Hooks : GameHooks {
                std::function<std::optional<formats::NpcInfo>(int npcId)> npcInfo;
                std::function<core::Size(const formats::NpcInfo&, int slot)> spriteSize;
                std::function<int(const formats::NpcInfo&, int slot)> frameCount;
            };

            /// @brief Creates the game with its callbacks and a random set of dolls (original TDollManage.Create).
            /// @param hooks Callbacks into the client.
            explicit DollGame(
                Hooks hooks
            );

            /// @brief Starts the game: claw home, dolls scattered, points asked for with 34/1 (original StartCatchDollGame).
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief A form button went down: asks to pay first when unpaid, else sets the claw move state (original TCatchDollForm.CatcherButtonDown).
            /// @param state Claw state the button stands for.
            void buttonDown(ClawCatcher::State state);

            /// @brief A form button went up: the claw stops moving (original TCatchDollForm.CatcherButtonUp).
            void buttonUp();

            /// @brief The Quit button (original QuitCatchDollClick).
            void quitClicked();

            /// @brief Leave on the instructions (original ExitCatchDoll).
            void leave() { exit(); }

            /// @brief Handles server packet 71/7: 1 and 2 answer the play request, 3 is the prize (RecvDollPlayError, RecvDollReward).
            /// @param payload Packet bytes.
            void receive(const core::Bytes& payload);

            /// @brief Runs the frames (30 ms each) that have passed (original UpdateCatchDollGame and DrawCatchDollGame).
            /// @param nowMs Current time in ms.
            /// @param instructionsOpen Whether the instructions are up (the form shows once they are not).
            void update(std::uint64_t nowMs, bool instructionsOpen);

            /// @brief Begin on the instructions.
            void begin() { m_confirmed = false; }

            /// @brief Whether the form is shown.
            /// @return True when shown.
            bool formShown() const { return m_formShown; }

            /// @brief The dolls.
            /// @return The doll array.
            const std::array<NpcDoll, kDolls>& dolls() const { return m_dolls; }

            /// @brief The claw.
            /// @return The claw object.
            const ClawCatcher& catcher() const { return m_catcher; }

            /// @brief How far the time has come from the last frame towards the next.
            /// @param nowMs Current time in ms.
            /// @return Fraction from 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Where the claw was along the top before the last frame.
            /// @return X coordinate.
            int previousClawX() const { return m_previousClawX; }

            /// @brief Size of a doll's sprite as it stands now (original GetBodySpriteWidth and GetBodySpriteHeight).
            /// @param doll Doll index.
            /// @return Sprite size.
            core::Size spriteSizeOf(int doll) const;

        private:
            /// @brief Runs one 30 ms frame: claw move, catch checks, doll poses and animation, claw drawing.
            /// @param nowMs Frame time in ms.
            void frame(std::uint64_t nowMs);

            /// @brief Moves every doll's sprite frame on by the clock (original DrawHumanSprite of each doll).
            /// @param nowMs Frame time in ms.
            void animateDolls(std::uint64_t nowMs);

            /// @brief Gives a doll a random kind and facing (original TDollManage.Create and RandomizeDolls).
            /// @param doll The doll to set up.
            void chooseKind(NpcDoll& doll);

            /// @brief Puts each doll on the floor, nearer ones in front (original ScatterDolls).
            void scatterDolls();

            /// @brief Orders the doll places by y so nearer ones come last (original SortDollsByY).
            void sortDollsByY();

            /// @brief Picks new kinds and places for all dolls (original RandomizeDolls).
            void randomizeDolls();

            /// @brief Fills the machine again when all dolls are down in the hole (original ResetDollsIfAllTaken).
            void resetDollsIfAllTaken();

            /// @brief Left edge of a doll for the catch test (original GetDollLeftEdge).
            /// @param doll Doll index, negative for none.
            /// @return X coordinate.
            int leftEdge(int doll) const;

            /// @brief Right edge of a doll for the catch test (original GetDollRightEdge).
            /// @param doll Doll index, negative for none.
            /// @return X coordinate.
            int rightEdge(int doll) const;

            /// @brief Shifts a doll left (original ShiftDollX).
            /// @param doll Doll index, -1 for none.
            /// @param delta Pixels to the left.
            void shiftX(int doll, int delta);

            /// @brief Shifts a doll up (original ShiftDollY).
            /// @param doll Doll index, -1 for none.
            /// @param delta Pixels up.
            void shiftY(int doll, int delta);

            /// @brief Lets a doll fall towards a y (original DropDollTowardY).
            /// @param doll Doll index.
            /// @param targetY Y to settle at.
            void dropTowardY(int doll, int targetY);

            /// @brief When the claw closes, settles which doll it holds and asks the server for the prize with 71/7 (original CheckClawCatch).
            void checkClawCatch();

            /// @brief Moves the held doll with the claw and finishes the catch at the hole (original UpdateDollCatch).
            void updateDollCatch();

            /// @brief Raises the held doll a little and keeps it below the closed claw (original ClampHeldDollToClaw).
            void clampHeldDollToClaw();

            /// @brief Makes the held doll follow the claw up by a moving-picture frame (original CalcHeldDollLiftStep).
            void calcHeldDollLiftStep();

            /// @brief Gives the doll held by the closed claw its hanging pose (original SetCaughtDollPose).
            void setCaughtDollPose();

            /// @brief Kind of the doll held in the middle of the claw (original GetCaughtDollPrize).
            /// @return Kind index, 0 for none.
            int caughtDollPrize() const;

            /// @brief Ends a catch: shows the result, resets the claw and asks to play again.
            /// @param message Result text.
            void finishCatch(const std::string& message);

            /// @brief Asks how to pay (original ChooseDollPayment).
            void chooseDollPayment();

            /// @brief Asks again in the main box for the way chosen from the list (original SelectDollPayment).
            /// @param choice 0 points, 1 Carnie Token, 2 Fun Token.
            void selectDollPayment(int choice);

            /// @brief Pays for a play (original PlayDollWithPoints, PlayDollWithCarnieToken, PlayDollWithFunToken).
            /// @param payKind 1 points, 2 Carnie Tokens, 3 Fun Tokens.
            void playWith(int payKind);

            /// @brief Refills dolls if needed, then asks for or takes the next play (original AskPlayDollAgain).
            void askPlayAgain();

            /// @brief Stops the music and ends the game (original ExitCatchDoll).
            void exit();

            /// @brief Exits, counting what the claw holds (original TSportCatchDoll.QuitCatchDollWithResult).
            void quitWithResult();

            /// @brief Whether the bag has room for a reward.
            /// @return True when there is a free place.
            bool hasRewardSpace() const { return m_hooks.freeBagPlaces() > 0; }

            /// @brief Random number below a count.
            /// @param count Exclusive upper bound (at least 1 is used).
            /// @return A value from 0 to count - 1.
            int random(int count) { return std::uniform_int_distribution<int>(0, std::max(count, 1) - 1)(m_random); }

            Hooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            ClawCatcher m_catcher;
            std::array<NpcDoll, kDolls> m_dolls;
            std::uint64_t m_lastFrameMs = 0;
            int m_previousClawX = ClawCatcher::kHomeX;
            bool m_formShown = false;
            bool m_paid = false;
            bool m_confirmed = false;
            bool m_catchRequestSent = false;
            int m_payKind = 0;
            std::string m_dollText;
    };

}

#endif
