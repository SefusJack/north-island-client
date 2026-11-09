#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_OVERLAYS_ROLE_INTRO_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_OVERLAYS_ROLE_INTRO_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <functional>
#include <string>

namespace client {

    /// @brief A character's introduction picture over the whole screen, opened from a star of the star book.
    /// @details Original: RoleIntroWindow.xml, TTH_SHowHumInfoForm. The player's own character for his own star, the character of a star's picture for the others.
    class RoleIntroWindow {
        public:
            /// @brief Callbacks naming the own player's look and what to do when the picture is put away.
            /// @details Original: HideCtrl: the sky shows again.
            struct Hooks {
                std::function<int()> bodyType;
                std::function<int()> head;
                std::function<void()> closed;
            };

            /// @brief Loads the layer and wires the painter and the back / right-click close handlers.
            /// @param services Shared game services.
            /// @param screen Screen the layer is loaded into.
            /// @param hooks Look and closed callbacks.
            RoleIntroWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the picture is showing.
            /// @return True when the layer is visible.
            bool isOpen() const;

            /// @brief Shows the own character's picture or a named one, in front of the other windows.
            /// @details Original: ShowCtrl with the own character's picture (LoadRolePic of his body type and head), or the named JRole picture.
            /// @param picture Name of a JRole picture; empty for the own character's.
            void open(const std::string& picture = "");

            /// @brief Hides the picture and calls the closed hook.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Draws the introduction picture and its overlay.
            /// @details Original: DrawShowHumInfo.
            /// @param canvas Canvas to draw on.
            void paint(gfx::Canvas& canvas);

            GameServices& m_services;
            ui::Screen& m_screen;
            Hooks m_hooks;
            ui::Element& m_layer;
            const ui::Spot& m_overlaySpot;
            std::string m_picture;
            std::string m_overlay;
    };

}

#endif
