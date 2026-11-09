#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FORTUNE_EGG_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_FORTUNE_EGG_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"

#include <cstdint>

namespace client {

    /// @brief The Fortune Egg game (original CY_LittleSport.pas TTrunEggGame and TTrunEggForm without drawing; game kind 6, 71/6).
    class FortuneEgg {
        public:
            static constexpr int kButtonFrames = 8; // frames of the button animation
            static constexpr int kButtonSize = 125; // width and height of a button picture frame, px
            static constexpr int kEggFrames = 14; // frames of the egg cracking animation
            static constexpr int kEggWidth = 193; // width of an egg picture frame, px
            static constexpr int kEggHeight = 116; // height of an egg picture frame, px
            static constexpr int kPointsPerPlay = 35; // IM Points one play costs
            static constexpr int kTokensPerPlay = 7; // tokens one play costs

            /// @brief Creates the game with its callbacks.
            /// @param hooks Callbacks into the client.
            FortuneEgg(
                GameHooks hooks
            );

            /// @brief Starts the game with nothing paid and the machine not yet active (original TTrunEggGame.Create).
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Begin on the instructions: activates the machine and asks for the points.
            void begin();

            /// @brief Leave on the instructions (original ExitTrunEggGame(0)).
            void leave();

            /// @brief The Exit button (original ExitTrunEggGame(0)).
            void exitClicked();

            /// @brief The round play button (original TTrunEggForm.PlayButtonClick, StartTrunEgg).
            void playClicked() { start(); }

            /// @brief Handles server packet 71/6: 1 the egg with its prize, 2 the points are short (original RecvTrunEggResult).
            /// @param payload Packet bytes.
            void receive(const core::Bytes& payload);

            /// @brief Runs the button and egg animation by the clock (original AnimateTrunEggResult).
            /// @param nowMs Current time in ms.
            void step(std::uint64_t nowMs);

            /// @brief Starts the music with the first frame of an active machine (original DrawTrunEggGame).
            void startMusicWhenActive();

            /// @brief Whether the machine is active.
            /// @return True when active.
            bool active() const { return m_active; }

            /// @brief Whether the Exit button shows.
            /// @return True when shown.
            bool exitShown() const { return m_exitShown; }

            /// @brief Whether the button animation runs.
            /// @return True while animating.
            bool buttonAnimating() const { return m_buttonAnimating; }

            /// @brief Whether the egg animation runs.
            /// @return True while animating.
            bool eggAnimating() const { return m_eggAnimating; }

            /// @brief Current button frame.
            /// @return Frame index.
            int buttonFrame() const { return m_buttonFrame; }

            /// @brief Current egg frame.
            /// @return Frame index.
            int eggFrame() const { return m_eggFrame; }

        private:
            /// @brief Asks how to pay (original ChooseTrunEggPayment).
            void choosePayment();

            /// @brief Asks to confirm the way chosen from the list (original AskPlayTrunEgg).
            /// @param choice 0 points, 1 Carnie Tokens, 2 Fun Tokens.
            void askToPay(int choice);

            /// @brief Confirms the payment if there is enough to play.
            void confirmPayment();

            /// @brief Declines the payment and allows asking again.
            void cancelPayment();

            /// @brief Checks the points or tokens for a play, showing a message and cancelling when short.
            /// @return True when there is enough.
            bool hasEnoughToPlay();

            /// @brief Shows a pending prize, stops the music and ends the game (original ExitTrunEggGame).
            void exitGame();

            /// @brief Text asking to pay with points.
            /// @return The question.
            std::string pointsQuestion() const;

            /// @brief Text asking to pay with tokens.
            /// @param tokens Tokens left.
            /// @return The question.
            std::string tokensQuestion(int tokens) const;

            /// @brief Starts a play: asks for payment or sends the play request 71/6 (original StartTrunEgg).
            void start();

            GameHooks m_hooks;
            bool m_musicOn = false;
            bool m_active = false;
            bool m_exitShown = true;
            bool m_askPayment = true;
            bool m_paid = false;
            bool m_startConfirmed = false;
            bool m_waitingResult = false;
            int m_payKind = 1;
            bool m_hasPrize = false;
            bool m_resultShown = false;
            bool m_buttonAnimating = false;
            bool m_eggAnimating = false;
            bool m_eggSoundPlayed = false;
            int m_buttonFrame = 3; // resting button frame
            int m_eggFrame = 0;
            std::uint64_t m_buttonMs = 0;
            std::uint64_t m_eggMs = 0;
            std::uint64_t m_holdUntilMs = 0;
            int m_prizeItem = 0;
            int m_prizeCount = 0;
    };

}

#endif
