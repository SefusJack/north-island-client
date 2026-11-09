#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_JIGSAW_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_JIGSAW_GAME_H

#include "client/subsystems/minigames/FramePicture.h"
#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/MachineTables.h"
#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <string>

namespace client {

    /// @brief The Jigsaw slot machines: three picture strips spin and stop on server-named stops.
    /// @details Original: MZ_MarioGame.pas TSportMario, TSportMarioSec and TSportMarioThird; game kinds 11, 12 and 14, packet 71/kind.
    class JigsawGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kStrips = 3; // strips of the picture (rows of pieces)

            /// @brief Creates a machine of a kind.
            /// @param kind Game kind (11, 12 or 14).
            /// @param hooks Callbacks to the host.
            JigsawGame(
                int kind,
                GameHooks hooks
            );

            /// @brief The machine's game kind.
            /// @return The kind.
            int kind() const { return m_kind; }

            /// @brief The item ids of the ten prizes (index 0: no prize).
            /// @return The prize table of this machine.
            const MachineTables::Prizes& prizes() const;

            /// @brief Opens the machine with the strips at their first frames.
            /// @param nowMs Current time in ms.
            /// @details Original: TSportMario.Create; sends 34/1 to ask for the points.
            void open(std::uint64_t nowMs);

            /// @brief Begins from the instructions.
            void begin();

            /// @brief Leaves the machine from the instructions.
            /// @details Original: ExitMarioGame.
            void leave() { exitGame(); }

            /// @brief Handles the form's Start button.
            /// @details Original: StartMarioClick, StartMarioSecClick, StartMarioThirdClick.
            void startClicked();

            /// @brief Handles the form's Quit button.
            /// @details Original: QuitMarioClick and its siblings.
            void quitClicked();

            /// @brief Handles a server packet for this machine.
            /// @param payload Packet body, starting with the sub id.
            /// @details Original: 71/kind; sub 1 and 2 the answer to the play request, 3 the spin's result, 4 its bingo variant, 5 no room for the prize.
            void receive(const core::Bytes& payload);

            /// @brief Runs the frames that have passed.
            /// @param nowMs Current time in ms.
            /// @param instructionsOpen Whether the instructions are showing.
            void update(std::uint64_t nowMs, bool instructionsOpen);

            /// @brief Whether the machine form is shown.
            /// @return True when shown.
            bool formShown() const { return m_formShown; }

            /// @brief The frame of a strip to show.
            /// @param strip Strip number 1 to 3.
            /// @return The frame index.
            int shownFrame(int strip) const { return m_shown[static_cast<std::size_t>(strip - 1)]; }

            /// @brief The Bingo picture's frame over the middle strip.
            /// @return The frame, or -1 when not shown.
            int bingoFrame() const { return m_bingoShown; }

            /// @brief The flash of the bingo's halves (the 'Ex' answer).
            /// @return Rows of the picture to show on the middle and bottom strips, or -1 when not shown.
            int flashRows() const { return m_flashShown ? m_flashRows : -1; }

        private:
            /// @brief Runs one frame.
            /// @param nowMs Current time in ms.
            void frame(std::uint64_t nowMs);

            /// @brief Advances the spinning strips.
            /// @param nowMs Current time in ms.
            void spinStrips(std::uint64_t nowMs);

            /// @brief Puts the strips back at their first frames.
            void resetStrips();

            /// @brief Starts a play.
            void startGame();

            /// @brief Closes the machine.
            void exitGame();

            /// @brief Sends the play request.
            void sendPlay();

            /// @brief Cost of a play in points.
            /// @return The point cost.
            int cost() const;

            /// @brief Cost of a play in tokens.
            /// @return The token cost.
            int tokenCost() const;

            /// @brief Asks which way to pay.
            void chooseWay();

            /// @brief Asks to confirm paying.
            /// @param choice The chosen way to pay.
            void askToPay(int choice);

            /// @brief Plays paying with points.
            void playWithPoints();

            /// @brief Plays paying with tokens.
            /// @param payKind Kind of token payment.
            void playWithTokens(int payKind);

            /// @brief Asks whether to play again.
            void askPlayAgain();

            /// @brief Whether the bag has room for a prize.
            /// @return True when a place is free.
            bool hasRewardSpace() const { return m_hooks.freeBagPlaces() > 0; }

            /// @brief Text showing the player's points.
            /// @return The text.
            std::string pointsText() const;

            /// @brief Question text for paying with points.
            /// @return The text.
            std::string questionPoints() const;

            /// @brief Question text for paying with tokens.
            /// @param payKind Kind of token payment.
            /// @return The text.
            std::string questionTokens(int payKind) const;

            int m_kind;
            GameHooks m_hooks;
            std::uint64_t m_lastFrameMs = 0;
            bool m_formShown = false;
            bool m_betPlaced = false;
            bool m_reelsStopped = false;
            bool m_started = false;
            bool m_resultReceived = false;
            bool m_rewardPending = false;
            bool m_prizeWon = false;
            bool m_bingoFlash = false;
            bool m_confirmed = false;
            bool m_reelSoundOn = true;
            int m_payKind = 1;
            int m_spinRounds = 0;
            int m_reelStage = 0;
            int m_frameCounter = 0;
            std::uint64_t m_reelSoundMs = 0;
            std::array<FramePicture, kStrips> m_strips;
            FramePicture m_bingo;
            std::array<int, kStrips + 1> m_symbols{};
            std::array<int, kStrips> m_shown{};
            int m_bingoShown = -1;
            bool m_flashOn = false;
            int m_flashTick = 0;
            bool m_flashShown = false;
            int m_flashRows = 0;
            std::string m_rewardText;
    };

}

#endif
