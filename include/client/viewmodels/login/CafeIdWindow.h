#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CAFE_ID_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CAFE_ID_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The internet cafe's number form: asks for the cafe number and sends it.
    /// @details Original: CafeIdWindow.xml, se_mainUI.pas TSe_CafeIDForm; opened by the server (55/1, and 55/2 2 after a refused number); OK sends the number with 55/1, Cancel closes the form and the connection; Escape does not close it (FCloseOnEsc cleared).
    class CafeIdWindow {
        public:
            /// @brief Builds the form and wires its controls.
            /// @param services Shared game services.
            /// @param screen Screen the form lives on.
            /// @param closeConnection Called on Cancel to close the connection.
            CafeIdWindow(
                GameServices& services,
                ui::Screen& screen,
                std::function<void()> closeConnection
            );

            /// @brief Whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form and focuses the number field.
            /// @details Original: ShowCtrl.
            void open();

        private:
            /// @brief Sends the number and hides the form.
            /// @details Original: HandleCafeIdOkClick; client 55/1 with the text.
            void send();

            /// @brief Hides the form and closes the connection.
            /// @details Original: HandleCafeIdCancelClick.
            void cancel();

            /// @brief Hides the form and empties the field.
            /// @details Original: HideCtrl.
            void hide();

            GameServices& m_services;
            ui::Screen& m_screen;
            std::function<void()> m_closeConnection;
            ui::Window& m_window;
            ui::TextBox& m_number;
    };

}

#endif
