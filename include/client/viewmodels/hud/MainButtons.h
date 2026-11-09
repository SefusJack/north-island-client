#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_MAIN_BUTTONS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_MAIN_BUTTONS_H

#include "client/services/GameServices.h"
#include "core/Rect.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

namespace client {

    /// @brief The main button row at the bottom right of the world.
    /// @details Original: MainButtons.xml, RE_MainBtnForm.
    class MainButtons {
        public:
            /// @brief Loads the button row from its layout and caches the Item button's bounds.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            MainButtons(
                GameServices& services,
                ui::Screen& screen
            )
                : m_panel(screen.load(services.game.layout("MainButtons.xml"), services.loadContext())),
                  m_item(screen.get<ui::ImageButton>("MainItem")),
                  m_retained(screen.get<ui::ImageButton>("MainRetained")),
                  m_alertSpot(screen.get<ui::Spot>("MainItemAlert")) {
                m_item.setAnimating(false);
                m_itemBounds = m_item.bounds();
            }

            /// @brief Shows the button row.
            void show() { m_panel.setVisible(true); }

            /// @brief Hides the button row.
            void hide() { m_panel.setVisible(false); }

            /// @brief Makes the Item button blink: it moves 2 pixels right and 8 down and plays its blinking strip (39 x 43 frames).
            /// @details Original: SetItemBtnAlert(0), used when stat points came.
            void alertItem();

            /// @brief Stops the Item button's blinking and restores its place.
            /// @details Original: SetItemBtnAlert(1).
            void clearItemAlert();

            /// @brief Shows or hides the "Pick up the furniture" button.
            /// @param shown Whether the button is visible.
            void showRetained(bool shown) { m_retained.setVisible(shown); }

        private:
            ui::Element& m_panel;
            ui::ImageButton& m_item;
            ui::ImageButton& m_retained;
            const ui::Spot& m_alertSpot;
            core::Rect m_itemBounds;
            bool m_alerting = false;
    };

}

#endif
