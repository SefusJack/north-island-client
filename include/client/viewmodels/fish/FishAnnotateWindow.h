#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_FISH_FISH_ANNOTATE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_FISH_FISH_ANNOTATE_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Window.h"

namespace client {

    /// @brief The fish tank's instructions: a picture with the text, opened by the tank's "?" button.
    /// @details Original: FishAnnotateWindow.xml, MK_Menu.pas TFishAnnotate.
    class FishAnnotateWindow {
        public:
            /// @brief Loads the layout and centres the window on the screen.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            FishAnnotateWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window in front.
            /// @details Original: ShowFishInstructions.
            void show();

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

        private:
            ui::Screen& m_screen;
            ui::Window& m_window;
    };

}

#endif
