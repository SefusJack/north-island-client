#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_GLOBE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_GLOBE_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Window.h"

namespace client {

    /// @brief Window of the world map of a house, with a hint button for each of its forty-one places.
    /// @details Original: GlobeWindow.xml, WS_Menu.pas TGlobaForm. The owner's click on the piece opens it; Close closes it, Escape does not (FCloseOnEsc is cleared).
    class GlobeWindow {
        public:
            static constexpr int kGlobeItem = 39092; // item 39092: Globe (item type 30)

            /// @brief Loads the window and wires its close button.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            GlobeWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window in front.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

        private:
            ui::Screen& m_screen;
            ui::Window& m_window;
    };

}

#endif
