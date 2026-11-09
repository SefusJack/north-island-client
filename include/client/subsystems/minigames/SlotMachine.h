#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SLOT_MACHINE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_SLOT_MACHINE_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief A slot machine of the game hall (game kinds 8, 10 and 19).
    /// @details Original: CY_LittleSport.pas TSlotmachGame, TSlotmach2Game, TSlotmach3Game and the parts of TTrunEggForm they use;
    /// they differ in prizes, play cost and question words. Server messages are 71/kind.
    class SlotMachine {
        public:
            static constexpr int kReels = 3; // reels, 1-based
            static constexpr int kReelFrames = 16; // symbols on a reel strip
            static constexpr int kReelHeight = 122; // height of one reel symbol, px
            static constexpr int kReelWidth = 127; // width of a reel, px
            static constexpr int kFlashHeight = 40; // height of the winning flash picture, px
            static constexpr int kFlashWidth = 202; // width of the winning flash picture, px

            /// @brief Creates a machine.
            /// @param kind Game kind (8, 10 or 19).
            /// @param hooks Host callbacks.
            SlotMachine(
                int kind,
                GameHooks hooks
            );

            /// @brief The machine's game kind.
            /// @return 8, 10 or 19.
            int kind() const { return m_kind; }

            /// @brief Resets the machine: reels on their first frame, nothing paid, form not yet active.
            /// @details Original: the constructor TSlotmachGame.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Makes the machine active (its form shows), after Begin on the instructions.
            void begin();

            /// @brief Leaves via the instructions' Leave.
            /// @details Original: GameExplainForm.LeaveGameClick, ExitSlotmachGame(0).
            void leave();

            /// @brief Handles the Exit button.
            void exitClicked();

            /// @brief Handles the Play button.
            /// @details Original: TTrunEggForm.PlayButtonClick.
            /// @param nowMs Current time in ms.
            void playClicked(std::uint64_t nowMs);

            /// @brief Handles a 71/kind message: 1 the reels and the prize, 2 the points are short.
            /// @param payload Packet bytes (code at offset 2).
            void receive(const core::Bytes& payload);

            /// @brief Runs the clock work of a frame.
            /// @details Original: AnimateSlotmachReels.
            /// @param nowMs Current time in ms.
            void step(std::uint64_t nowMs);

            /// @brief Whether the Lucky Star button shows.
            /// @details Original: LuckyStarClick's button.
            /// @return True when shown.
            bool luckyStarShown() const { return m_luckyStarShown; }

            /// @brief Whether the Lucky Star is to be used.
            /// @return True when on.
            bool luckyStarOn() const { return m_luckyStarOn; }

            /// @brief Toggles the Lucky Star.
            /// @param explicitClick True for a click on its button, false when the machine finds the star used up.
            void luckyStarClicked(bool explicitClick = true);

            /// @brief Whether the machine is active.
            /// @return True when active.
            bool active() const { return m_active; }

            /// @brief Whether the Exit button shows.
            /// @return True when shown.
            bool exitShown() const { return m_exitShown; }

            /// @brief Whether the music is on.
            /// @return True when on.
            bool musicOn() const { return m_musicOn; }

            /// @brief Frame position of a reel.
            /// @param reel Reel number 1 to kReels.
            /// @return The frame 1 to kReelFrames.
            int reelPosition(int reel) const { return m_reelPos[static_cast<std::size_t>(reel)]; }

            /// @brief Whether the prize flash is on.
            /// @return True while flashing.
            bool prizeFlash() const { return m_prizeFlash; }

            /// @brief Frame of the prize flash.
            /// @return The frame 1 or 2.
            int flashFrame() const { return m_flashFrame; }

            /// @brief The prize slot the server named.
            /// @return The slot (0: none).
            int prizeSlot() const { return m_prizeSlot; }

            /// @brief How far the banner has slid down.
            /// @details Original: DrawSlidingText.
            /// @param nowMs Current time in ms.
            /// @return Pixels slid (0 while at rest).
            int bannerSlide(std::uint64_t nowMs);

            /// @brief Starts the music with the first frame of an active machine.
            /// @details Original: DrawSlotmachGame.
            void startMusicWhenActive();

            /// @brief What a play costs in points.
            /// @return The point cost.
            int pointCost() const;

            /// @brief Whether a play is free.
            /// @details Original: Player.FNewYearEventOn and FLucky7Count >= 7.
            /// @return True when free.
            bool freePlay() const;

            /// @brief What a play costs in tokens.
            /// @return The token cost.
            int tokenCost() const;

        private:
            /// @brief Starts a play: checks the bag, asks how to pay, then sends the play.
            /// @details Original: StartSlotmach.
            /// @param nowMs Current time in ms.
            void start(std::uint64_t nowMs);

            /// @brief Asks how to pay.
            /// @details Original: ChooseSlotmachPayment.
            void choosePayment();

            /// @brief Asks for confirmation of the chosen payment.
            /// @details Original: AskPlaySlotmach.
            /// @param choice 0 points, 1 Carnie Tokens, 2 Fun Tokens.
            void askToPay(int choice);

            /// @brief Confirms the payment.
            /// @details Original: TTrunEggForm.ConfirmPlayPayment.
            void confirmPayment();

            /// @brief Cancels the payment.
            /// @details Original: TTrunEggForm.CancelPlayPayment.
            void cancelPayment();

            /// @brief Checks the player can pay for a play.
            /// @details Original: HasEnoughPlayCurrency of TTrunEggForm.ConfirmPlayPayment.
            /// @return True when enough.
            bool hasEnoughToPlay();

            /// @brief Leaves the machine.
            /// @details Original: ExitSlotmachGame.
            void exitGame();

            /// @brief Tells the player of the prize obtained.
            /// @param withCount Whether to name the quantity.
            void showObtained(bool withCount);

            /// @brief Words of the question when paying with points.
            /// @return The question text.
            std::string pointsQuestion() const;

            /// @brief Words of the question when paying with tokens.
            /// @param tokens Tokens the player has.
            /// @return The question text.
            std::string tokensQuestion(int tokens) const;

            /// @brief Sends the play to the server.
            /// @details Original: SendServerMsg(71, kind) with the way to pay and the Lucky Star place.
            void sendPlay();

            /// @brief Looks for a Lucky Star in the bag and offers it.
            /// @details Original: TTrunEggForm.ShowCtrl.
            void findLuckyStar();

            int m_kind;
            GameHooks m_hooks;
            bool m_musicOn = false;
            bool m_active = false;
            bool m_exitShown = true;
            bool m_askPayment = true;
            bool m_paid = false;
            bool m_startConfirmed = false;
            int m_payKind = 1;
            bool m_spinning = false;
            bool m_spinSoundPending = false;
            bool m_resultShown = false;
            bool m_wonPrize = false;
            bool m_prizeKnown = false;
            bool m_prizeFlash = false;
            int m_flashFrame = 1;
            std::uint64_t m_flashStartMs = 0;
            std::uint64_t m_flashMs = 0;
            std::uint64_t m_spinStartMs = 0;
            std::uint64_t m_reelMs = 0;
            std::uint64_t m_bannerStartMs = 0;
            std::array<int, kReels + 1> m_reelPos{};
            std::array<int, kReels + 1> m_reelTarget{};
            std::array<bool, kReels + 1> m_reelStopped{};
            int m_reelIntervalMs = 50; // milliseconds between reel steps
            int m_prizeSlot = 0;
            int m_prizeItem = 0;
            int m_prizeCount = 0;
            bool m_luckyStarShown = false;
            bool m_luckyStarOn = false;
            int m_luckyStarPlace = 0;
    };

}

#endif
