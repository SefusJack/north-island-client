#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MARRIAGE_MARRY_DIVORCE_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MARRIAGE_MARRY_DIVORCE_FORM_H

#include "client/services/GameServices.h"
#include "client/models/social/Marriage.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

namespace client {

    /// @brief Form for a divorce application the server opened for a couple.
    /// @details Original: MarryDivorceForm.xml, TMarryDivorce; opened by 45/23. Divorce accepts and waits for the
    /// other one; Close turns it down.
    class MarryDivorceForm {
        public:
            /// @brief Loads the form layout and wires its buttons and painter.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param marriage Marriage state holding the couple.
            MarryDivorceForm(
                GameServices& services,
                ui::Screen& screen,
                Marriage& marriage
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with the Divorce button, not the waiting picture.
            /// @details Original: ShowMarryDivorce.
            void open();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            GameServices& m_services;
            ui::Screen& m_screen;
            Marriage& m_marriage;
            ui::Window& m_window;
            ui::ImageButton& m_divorce;
            ui::Element& m_waiting;
            const ui::Spot& m_headingSpot;
    };

}

#endif
