#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_RELOGIN_SCREEN_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_RELOGIN_SCREEN_H

#include "client/services/GameServices.h"
#include "ui/Element.h"
#include "ui/ProgressBar.h"
#include "ui/Screen.h"

namespace client {

    /// @brief Loading screen of a relogin; mode 0 fills a bar, mode 1 (PvP server) shows the PvP mark instead.
    /// @details Original: ReloginScreen.xml, DrawLoadingScreen; the bar of mode 0 fills a step a frame to 30.
    class ReloginScreen {
        public:
            /// @brief Loads ReloginScreen.xml.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            ReloginScreen(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the screen is shown.
            /// @return True when visible.
            bool isVisible() const { return m_layer.isVisible(); }

            /// @brief Shows the screen with the bar and logos, or the PvP mark.
            /// @param pvp True for mode 1 (the PvP server).
            void show(bool pvp);

            /// @brief Hides the screen.
            void hide();

            /// @brief Advances the loading bar one step per frame, 30 at most.
            /// @details Original: FLoadProgress.
            void update();

        private:
            ui::Screen& m_screen;
            ui::Element& m_layer;
            ui::ProgressBar& m_progress;
    };

}

#endif
