#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_MESSAGE_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_MESSAGE_FORM_H

#include "client/services/GameServices.h"
#include "core/Point.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The form that asks for the creator's message when creating an instance; Confirm hands the message on, Cancel and close forget the text.
    /// @details Original: InstanceMessageForm.xml, TEnterDupMsgForm; the create form opens it beside itself.
    class InstanceMessageForm {
        public:
            /// @brief Loads the form and wires its events.
            /// @details Original: onConfirm is SendCreateDup with the typed message.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param onConfirm Called with the typed message on Confirm.
            InstanceMessageForm(
                GameServices& services,
                ui::Screen& screen,
                std::function<void(const std::string&)> onConfirm
            );

            /// @brief Tells whether the form is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form at a place; the memo gets the keyboard.
            /// @details Original: ShowCtrl.
            /// @param at Top-left of the form.
            void open(core::Point at);

            /// @brief Moves the form (the create form keeps it beside itself) and fits it to the screen.
            /// @details Original: PlaceEnterDupMsgForm.
            /// @param at Top-left of the form.
            void place(core::Point at);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

            /// @brief Empties the message text.
            /// @details Original: ClearMemoText.
            void clearText() { m_text.clear(); }

        private:
            /// @brief Closes the form and forgets the text.
            /// @details Original: CancelEnterDupMsg.
            void cancel();

            /// @brief Hands the trimmed message on and closes the form.
            /// @details Original: ConfirmEnterDupMsg.
            void confirm();

            ui::Screen& m_screen;
            std::function<void(const std::string&)> m_confirm;
            ui::Window& m_window;
            ui::Memo& m_text;
    };

}

#endif
