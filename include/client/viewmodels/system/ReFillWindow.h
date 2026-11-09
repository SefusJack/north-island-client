#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_RE_FILL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_RE_FILL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief Prepaid card form with a card number and a password.
    /// @details Original: ReFillWindow.xml, se_mainUI.pas TSe_ReFillForm. Opened by the server (35/6) and closed by it (35/11); OK first asks whether there is room for gifts, then sends 35/3. While the server has not been answered the form does not close, it only says so.
    class ReFillWindow {
        public:
            using Ask = std::function<void(const std::string& text, std::function<void()> onOk)>;

            /// @brief Loads ReFillWindow.xml and wires the OK, Cancel, Close and Next events.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param messages Message line used for refusals and hints.
            /// @param ask Callback that asks the gift-space question.
            ReFillWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Ask ask
            );

            /// @brief Tells whether the form is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form after the server's 35/6; closing is refused until it is answered.
            /// @details Original: ShowReFillForm.
            void open();

            /// @brief Empties and closes the form once the server has answered (35/11).
            /// @details Original: CloseReFillForm.
            void close();

        private:
            /// @brief Hides the form, or says so when the server's question still stands.
            /// @details Original: HideCtrl.
            void hide();

            /// @brief Empties the card and password fields.
            void clear();

            /// @brief Asks whether there is room for gifts, then submits on confirmation.
            /// @details Original: HandleReFillOkClick.
            void ask();

            /// @brief Sends the card number and password (35/3) when both are filled in, else asks for the missing one.
            /// @details Original: SubmitReFillCard.
            void submit();

            /// @brief Moves focus on from a field: card to password, password to the gift question.
            /// @details Original: FocusNextReFillEdit.
            /// @param field Name of the field that asked for the next one.
            void next(const std::string& field);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Ask m_ask;
            ui::Window& m_window;
            bool m_mayClose = true;
    };

}

#endif
