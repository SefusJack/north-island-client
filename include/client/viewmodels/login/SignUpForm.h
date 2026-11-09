#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_SIGN_UP_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_SIGN_UP_FORM_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The qol client's sign-up page: an account and a password typed twice; the server makes the account on first login.
    /// @details Original: the login form's Sign Up opened a web page; this page is qol-only (SignUpForm.xml).
    class SignUpForm {
        public:
            /// @brief Hooks the form calls for its buttons.
            struct Callbacks {
                std::function<void(const std::string& account, const std::string& password)> signUp;
                std::function<void()> back;
            };

            /// @brief Loads SignUpForm.xml and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Short player message line.
            /// @param callbacks Sign-up and back hooks.
            SignUpForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Callbacks callbacks
            );

            /// @brief Shows the form; the account typed so far stays (after a refusal) and the passwords are emptied.
            /// @param account Account to put in the field.
            void show(const std::string& account = {});

            /// @brief Hides the form and empties the passwords.
            void hide();

            /// @brief Tells whether the form is shown.
            /// @return True when visible.
            bool isVisible() const { return m_form.isVisible(); }

        private:
            /// @brief Checks the fields and reports the sign-up.
            void submit();

            /// @brief Moves the keyboard to the next field after Enter or Tab.
            /// @param from Name of the field that had the keyboard.
            void next(const std::string& from);

            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Callbacks m_callbacks;
            ui::Element& m_form;
            ui::TextBox& m_account;
            ui::TextBox& m_password;
            ui::TextBox& m_again;
    };

}

#endif
