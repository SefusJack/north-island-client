#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MARRIAGE_MARRY_APPLICATION_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MARRIAGE_MARRY_APPLICATION_FORM_H

#include "client/services/GameServices.h"
#include "client/models/social/Marriage.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Window.h"

namespace client {

    /// @brief Form for a marriage application the server opened for two players.
    /// @details Original: MarryApplicationForm.xml, TMarryApplication; opened by 45/1. Marry accepts and waits
    /// for the other one; Close turns it down.
    class MarryApplicationForm {
        public:
            /// @brief Loads the form layout and wires its buttons and painter.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param marriage Marriage state holding the applicants.
            MarryApplicationForm(
                GameServices& services,
                ui::Screen& screen,
                Marriage& marriage
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with the Marry button, not the waiting picture.
            /// @details Original: ShowMarryApplication.
            void open();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            GameServices& m_services;
            ui::Screen& m_screen;
            Marriage& m_marriage;
            ui::Window& m_window;
            ui::ImageButton& m_marry;
            ui::Element& m_waiting;
    };

}

#endif
