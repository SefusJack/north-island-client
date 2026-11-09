#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_ID_AND_PW_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_ID_AND_PW_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <string>

namespace client {

    /// @brief Second ID and password window opened by the server; account must be "WL" plus a number, password typed as stars.
    /// @details Original: IdAndPwWindow.xml, RE_Menu.pas TRe_IdAndPwForm; opened by server 21/1 and 21/2 (mode kept in FMode); OK sends 21/mode.
    class IdAndPwWindow {
        public:
            /// @brief Loads IdAndPwWindow.xml and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Short player message line.
            IdAndPwWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages
            );

            /// @brief Tells whether the window is shown.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Remembers the mode, empties both fields and shows the window.
            /// @details Original: SetIdAndPwMode, ShowCtrl.
            /// @param mode 1 or 2, the server arm (21/1 or 21/2) that opened the form.
            void open(int mode);

        private:
            /// @brief Checks the fields and sends the request, or empties the account and says why.
            /// @details Original: ConfirmIdAndPw.
            void confirm();

            /// @brief Checks the field that had the keyboard, then moves on (past the last one the form is confirmed).
            /// @details Original: FocusNextIdPwEditor.
            /// @param field Name of the field that had the keyboard.
            void next(const std::string& field);

            /// @brief Tells whether the account is "WL" followed by a whole number.
            /// @param account Typed account.
            /// @return True when valid.
            static bool validAccount(const std::string& account);

            /// @brief Hides the window.
            void hide() { m_window.setVisible(false); }

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            int m_mode = 0;
    };

}

#endif
