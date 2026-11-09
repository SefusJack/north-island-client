#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_OVERLAYS_STAR_SKY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_OVERLAYS_STAR_SKY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/rendering/effects/StarSky.h"
#include "client/viewmodels/overlays/RoleIntroWindow.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"

namespace client {

    /// @brief The sky of the star book and the flight of a new star, over the map; paints what the star system says and takes the mouse.
    /// @details Original: StarSkyWindow.xml, CH_StarSystem and TSe_StarSysExit.
    class StarSkyWindow {
        public:
            /// @brief Loads the layer and wires the painter, click and Quit handlers.
            /// @param services Shared game services.
            /// @param screen Screen the layer is loaded into.
            /// @param sky The star system that is painted and clicked.
            /// @param intro Introduction picture window opened from a star.
            StarSkyWindow(
                GameServices& services,
                ui::Screen& screen,
                StarSky& sky,
                RoleIntroWindow& intro
            );

            /// @brief Shows the layer while the star system is at work.
            void update();

            /// @brief Shows or hides the Quit button.
            /// @details Original: TSe_StarSysExit.ShowCtrl / HideCtrl.
            /// @param shown True to show the button.
            void setQuitShown(bool shown);

        private:
            /// @brief Opens the introduction of the star under the mouse (own star or a star's picture).
            /// @details Original: OpenStarRoleInfo: a left click on a star opens its introduction.
            void openIntroduction();

            ui::Screen& m_screen;
            StarSky& m_sky;
            RoleIntroWindow& m_intro;
            ui::Element& m_layer;
            ui::ImageButton& m_quit;
    };

}

#endif
