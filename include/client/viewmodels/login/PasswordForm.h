#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_PASSWORD_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_PASSWORD_FORM_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"

#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Password and cipher form, asked before deleting a character (Confirm) and at the end of character creation (Set).
    /// @details Original: PasswordForm.xml. The cipher is typed twice and must be 6..10 characters.
    class PasswordForm {
        public:
            /// @brief Why the form is asked for.
            enum class Mode { Set, Confirm };

            /// @brief What the player entered.
            struct Result {
                std::string password;
                std::string cipher;
            };

            /// @brief Loads PasswordForm.xml and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Short player message line.
            PasswordForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages
            );

            /// @brief Empties the fields and shows the form in a mode.
            /// @param mode Set (new cipher) or Confirm (existing cipher).
            /// @param onOk Called with the entered values when OK passes the checks.
            /// @param onCancel Called when the player cancels.
            void show(Mode mode, std::function<void(const Result&)> onOk, std::function<void()> onCancel);

            /// @brief Hides the form.
            void hide() { m_form.setVisible(false); }

            /// @brief Tells whether the form is shown.
            /// @return True when visible.
            bool isVisible() const { return m_form.isVisible(); }

        private:
            /// @brief Moves the keyboard to the next field; after the last one acts as OK.
            void focusNext();

            /// @brief Checks both pairs of fields and the cipher length, then reports OK.
            void confirm();

            /// @brief Hides the form and reports the cancel.
            void cancel();

            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            ui::Element& m_form;
            ui::Element& m_setMode;
            ui::Element& m_confirmMode;
            std::vector<std::reference_wrapper<ui::TextBox>> m_fields;
            int m_focus = 0;
            std::function<void(const Result&)> m_onOk;
            std::function<void()> m_onCancel;
    };

}

#endif
