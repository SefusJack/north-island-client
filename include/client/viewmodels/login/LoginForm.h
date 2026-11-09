#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_LOGIN_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_LOGIN_FORM_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/CheckBox.h"
#include "ui/ComboBox.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"

#include <functional>
#include <string>

namespace client {

    /// @brief Account form with account history, password and "remember account".
    /// @details Original: LoginForm.xml; remembered accounts live in user\AccountList.dat.
    class LoginForm {
        public:
            /// @brief Hooks the form calls for its buttons.
            struct Callbacks {
                std::function<void(const std::string& account, const std::string& password)> login;
                std::function<void()> previous;
                std::function<void()> signUp;
            };

            /// @brief Loads LoginForm.xml and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Short player message line.
            /// @param callbacks Login, previous and sign-up hooks.
            LoginForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Callbacks callbacks
            );

            /// @brief Shows the form, clears the password, loads the account history and focuses the account.
            void show();

            /// @brief Hides the form and clears the password.
            void hide();

            /// @brief Empties the account and password after the server turned the login down.
            /// @details Original: SetEditorText(.., '') of both fields.
            void clearFields();

            /// @brief Tells whether the form is shown.
            /// @return True when visible.
            bool isVisible() const { return m_form.isVisible(); }

            /// @brief After a successful login, keeps the account in the list when the player asked for it.
            void rememberAccount();

            /// @brief After a sign-up, puts the new account into the list and the field.
            /// @param account The new account name.
            void rememberAccount(const std::string& account);

        private:
            /// @brief Checks the typed account (4..14 letters or digits) and complains when wrong.
            /// @return True when valid.
            bool checkAccount();

            /// @brief Validates the fields and reports the login.
            void submit();

            /// @brief Loads the remembered accounts from the account list file.
            void loadHistory();

            /// @brief Saves the remembered accounts, or removes the file when none are left.
            void saveHistory();

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Callbacks m_callbacks;
            ui::Element& m_form;
            ui::ComboBox& m_account;
            ui::TextBox& m_password;
            ui::CheckBox& m_remember;
    };

}

#endif
