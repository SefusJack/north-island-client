#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_LUCKY_CARDS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_LUCKY_CARDS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/LuckyCards.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Lucky Cards' table: green table, cards in two rows, the click-to-play picture, the sign of a pair or a miss and the Exit button, over a LuckyCards.
    /// @details Original: LuckyCardsWindow.xml, DrawPokeCards and TPokeForm.
    class LuckyCardsWindow {
        public:
            /// @brief Loads the form layout and wires its events.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Callbacks handed to the LuckyCards.
            /// @param instructionsOpen Tells whether the instructions are showing.
            LuckyCardsWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks,
                std::function<bool()> instructionsOpen
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives access to the game state.
            /// @return The LuckyCards behind the window.
            LuckyCards& cards() { return m_cards; }

            /// @brief Starts the game and shows the table under the instructions.
            /// @details Original: TPoke.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Begins the game and shows the Exit button.
            void begin();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Steps the game while the form is showing.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

        private:
            /// @brief Draws the table, the cards, the hints and the result sign.
            /// @details Original: DrawPokeCards.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the drawing area (unused).
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Handles a click: a card turns it; elsewhere on the table it starts a play.
            /// @details Original: SportMouseDown for a click off the cards.
            /// @param args Event arguments with the click position.
            void clicked(const ui::EventArgs& args);

            /// @brief Gives the top left of a card.
            /// @param card Card number, 1 to 8.
            /// @return Top left of the card on the screen.
            core::Point cardAt(int card) const;

            /// @brief Finds the visible card under a point.
            /// @param point Point on the screen.
            /// @return The card number, or 0 for none.
            int cardAtPoint(core::Point point) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            std::function<bool()> m_instructionsOpen;
            LuckyCards m_cards;
            ui::Element& m_form;
            ui::ImageButton& m_exit;
            const ui::Spot& m_cardSpot;
            const ui::Spot& m_faceSpot;
            const ui::Spot& m_resultSpot;
            const ui::Spot& m_hintSpot;
    };

}

#endif
