#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CHANGE_PASSWORD_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CHANGE_PASSWORD_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The change password form: old password and the new one twice, checked, then a question.
    /// @details Original: ChangePasswordWindow.xml, TSe_ChangePassWordForm; client 35/1 sends the change, server 35/1 answers.
    class ChangePasswordWindow {
        public:
            using Ask = std::function<void(const std::string& text, std::function<void()> onOk, std::function<void()> onCancel)>;

            /// @brief Builds the form and wires its controls.
            /// @param services Shared game services.
            /// @param screen Screen the form lives on.
            /// @param messages Player message sink.
            /// @param ask Shows the confirming question (MainTalkMsgForm, screen darkened) with OK and Cancel actions.
            ChangePasswordWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Ask ask
            );

            /// @brief Whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with empty fields (stars in the first three).
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Shows how the change went.
            /// @param result Result code from server 35/1.
            void showResult(int result);

            /// @brief Shows how the account password change went.
            /// @param result Result code from server 35/12.
            void showAccountResult(int result);

        private:
            /// @brief Checks the fields and asks for confirmation before sending 35/1.
            /// @details Original: SubmitChangePassword, ConfirmChangePassword.
            void submit();

            /// @brief Moves to the next field; past the last field it submits.
            /// @param field Name of the field that sent Enter or Tab.
            /// @details Original: FocusNextChangePasswordEdit.
            void next(const std::string& field);

            /// @brief Text of a field.
            /// @param field Field number 1..6.
            /// @return The text.
            std::string text(int field);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Ask m_ask;
            ui::Window& m_window;
    };

}

#endif
