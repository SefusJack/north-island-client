#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_PARTY_LOGIN_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_PARTY_LOGIN_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <string>

namespace client {

    /// @brief The qol client's form that logs another account in as a party member (PartyLoginWindow.xml).
    /// @details Reached from the group window's Log in, with the Video tab's "Log in more accounts" on: the account, its password and the second character (members always take manual battle commands). OK hands them to GameServices::partyLogIn and closes, or shows why not under the fields. Nothing typed is kept.
    class PartyLoginWindow {
        public:
            /// @brief Builds the form.
            /// @param services Shared game services.
            /// @param screen Screen the window is added to.
            PartyLoginWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the form is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form with empty fields and the account's field focused.
            void open();

            /// @brief Clears the password and hides the form.
            void close();

        private:
            /// @brief Hands the typed values to GameServices::partyLogIn and closes, or shows the error.
            void submit();

            /// @brief Moves on from a field: Enter or Tab in the account goes to the password; in the password it submits.
            /// @param field Name of the field the key was pressed in.
            void next(const std::string& field);

            GameServices& m_services;
            ui::Screen& m_screen;
            ui::Window& m_window;
    };

}

#endif
