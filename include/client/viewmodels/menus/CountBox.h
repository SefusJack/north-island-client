#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_COUNT_BOX_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_COUNT_BOX_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief Small box to change or type a number and hand it to whoever asked.
    /// @details Original: CountBox.xml, TCY_ThrowThingNumMenu. OK hands the number to the asker, who says whether the
    /// box may go (ConfirmThrowCount leaves it open when it refuses). Something not a number becomes 0 and the box goes.
    class CountBox {
        public:
            using Confirm = std::function<bool(int count)>;

            /// @brief Loads the box layout and wires its buttons.
            /// @param services Shared game services.
            /// @param screen Screen the box is loaded into.
            CountBox(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the box is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the box with a starting number.
            /// @details Original: AskStallBuyCount and the others; ShowCtrl shows "1" when the text is empty.
            /// @param count Number to begin with.
            /// @param confirm Called with the number on OK; returns true when the box may close.
            void ask(int count, Confirm confirm);

            /// @brief Opens the box keeping the number it had.
            /// @details Original: AskNpcBuyCount.
            /// @param confirm Called with the number on OK; returns true when the box may close.
            void ask(Confirm confirm);

            /// @brief Hides the box.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Hands the typed number to the asker and closes unless it refuses.
            /// @details Original: ConfirmThrowCount.
            void confirm();

            /// @brief Changes the number by a step, never below 0; an empty box counts as 0.
            /// @details Original: IncreaseThrowCount / DecreaseThrowCount.
            /// @param by Amount to add (1 or -1).
            void step(int by);

            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::TextBox& m_count;
            Confirm m_confirm;
    };

}

#endif
