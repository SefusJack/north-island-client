#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_LUCKY_CARDS_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_LUCKY_CARDS_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The Lucky Cards game: eight cards, two tries of two, a pair is a prize.
    /// @details Original: LD_PokeGame.pas TPoke and TPokeForm without their drawing; game kind 16; 57/9/6 deals, 57/9/3 turns a card, 57/9/4 shows every face.
    class LuckyCards {
        public:
            static constexpr int kCards = 8; // cards on the table
            static constexpr int kPointsPerPlay = 20; // IM Points one play costs
            static constexpr int kTokensPerPlay = 4; // tokens one play costs
            static constexpr std::uint64_t kTickMs = 30; // one game tick, milliseconds

            /// @brief Creates the game.
            /// @param hooks Callbacks to the host.
            LuckyCards(
                GameHooks hooks
            );

            /// @brief Opens the game with nothing dealt and the cards away.
            /// @param nowMs Current time in ms.
            /// @details Original: TPoke.Create.
            void open(std::uint64_t nowMs);

            /// @brief Shows the table from the form's Begin.
            /// @details Original: PokeForm.ShowCtrl.
            void begin();

            /// @brief Handles a 57/9 game packet.
            /// @param payload Packet body.
            /// @param nowMs Current time in ms.
            /// @details Original: 57/9 (RecvPokePacket).
            void receive(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Handles a 71/16 machine packet.
            /// @param payload Packet body.
            /// @details Original: 71/16 (RecvPokePlayError, RecvPokeReward).
            void receiveMachine(const core::Bytes& payload);

            /// @brief Runs the frames that have passed.
            /// @param nowMs Current time in ms.
            /// @details Original: UpdatePokeGame, once per 30 ms frame.
            void update(std::uint64_t nowMs);

            /// @brief Handles a click on the table.
            /// @details Original: SportMouseDown.
            void tableClicked();

            /// @brief Handles a click on a card.
            /// @param card Card number 1 to 8.
            /// @details Original: TPokeForm.FlipPokeCard.
            void cardClicked(int card);

            /// @brief Handles the Quit button.
            /// @details Original: TPokeForm.QuitPokeClick.
            void quitClicked();

            /// @brief Leaves from the instructions.
            /// @details Original: TPoke.ExitPokeGame.
            void leave() { exitGame(); }

            /// @brief Whether a play is in progress.
            /// @return True while playing.
            bool playing() const { return m_playing; }

            /// @brief Whether a mouse press on the table has been seen.
            /// @return True after the first press.
            bool mouseDownSeen() const { return m_mouseDownSeen; }

            /// @brief Whether the cards are dealt.
            /// @return True once dealt.
            bool cardsDealt() const { return m_cardsDealt; }

            /// @brief Whether a card turn waits for the server.
            /// @return True while pending.
            bool flipPending() const { return m_flipPending; }

            /// @brief Whether the last two turned cards matched.
            /// @return True for a pair.
            bool pairMatched() const { return m_pairMatched; }

            /// @brief Plays the pair result sound once, when the picture is first drawn.
            /// @details Original: FPairChecked.
            void soundResult();

            /// @brief Number of cards turned.
            /// @return The count.
            int flippedCount() const { return m_flippedCount; }

            /// @brief The card being turned.
            /// @return Card number.
            int flippingCard() const { return m_flippingCard; }

            /// @brief Offset of the turn animation.
            /// @return The offset.
            int flipOffset() const { return m_flipOffset; }

            /// @brief The turn's offset before the last update; with free frames the card is drawn between the two.
            /// @return The previous offset.
            int previousFlipOffset() const { return m_previousFlipOffset; }

            /// @brief When the last update ran.
            /// @return Time in ms.
            std::uint64_t updateMs() const { return m_updateMs; }

            /// @brief Number of cards on the table.
            /// @return The count.
            int cardCount() const { return m_cardCount; }

            /// @brief Whether a card is shown.
            /// @param card Card number.
            /// @return True when visible.
            bool cardVisible(int card) const { return m_visible[static_cast<std::size_t>(card)]; }

            /// @brief The face value of a card.
            /// @param card Card number.
            /// @return The value.
            int cardValue(int card) const { return m_values[static_cast<std::size_t>(card)]; }

            /// @brief When the turn animation last ticked.
            /// @return Time in ms.
            std::uint64_t flipTickMs() const { return m_flipTickMs; }

        private:
            /// @brief Resets the game state.
            /// @details Original: ResetPokeGame.
            void reset();

            /// @brief Shows or hides all cards.
            /// @param visible Whether to show them.
            void setCardsVisible(bool visible);

            /// @brief Deals the cards one at a time.
            /// @details Original: DealPokeCards.
            void deal();

            /// @brief Advances the card turn animation.
            void flipAnimation();

            /// @brief Judges the two turned cards a moment after the turn.
            /// @param nowMs Current time in ms.
            void judgeFlips(std::uint64_t nowMs);

            /// @brief Checks whether the turned cards are a pair.
            /// @param flipped How many cards are turned.
            /// @details Original: CheckPokePairMatch.
            void checkPair(int flipped);

            /// @brief Asks which way to pay.
            /// @details Original: ChoosePokePayment.
            void choosePayment();

            /// @brief Asks to confirm the chosen payment.
            /// @param choice The chosen way.
            /// @details Original: AskPlayPoke.
            void askToPay(int choice);

            /// @brief Pays and starts the play.
            /// @param kind Payment kind.
            /// @details Original: PlayPokeWith*.
            void pay(int kind);

            /// @brief Whether the bag has two free places.
            /// @return True when it has.
            bool hasTwoFreeBagPlaces() const;

            /// @brief Closes the game.
            /// @details Original: ExitPokeGame.
            void exitGame();

            /// @brief Quits the play in progress.
            /// @details Original: QuitPokeGame.
            void quitGame();

            GameHooks m_hooks;
            std::uint64_t m_lastTickMs = 0;
            int m_tick = 0;
            bool m_playing = false;
            int m_payKind = 0;
            bool m_quitting = false;
            bool m_flipLocked = false;
            bool m_mouseDownSeen = false;
            bool m_cardsDealt = false;
            int m_flipOffset = 0;
            int m_previousFlipOffset = 0;
            std::uint64_t m_updateMs = 0;
            int m_flippingCard = 0;
            bool m_flipPending = false;
            std::array<int, 5> m_flippedValues{};
            bool m_pairMatched = false;
            bool m_pairChecked = false;
            std::uint64_t m_flipTickMs = 0;
            int m_cardCount = 0;
            int m_flippedCount = 0;
            bool m_flipAnimating = false;
            bool m_flipHalfDone = false;
            std::array<bool, kCards + 1> m_visible{};
            std::array<int, kCards + 1> m_values{};
    };

}

#endif
