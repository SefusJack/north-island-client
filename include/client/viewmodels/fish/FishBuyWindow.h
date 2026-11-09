#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_FISH_FISH_BUY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_FISH_FISH_BUY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/fish/TankFish.h"
#include "client/viewmodels/fish/FishTankWindow.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief The fish tank's buy and discard form beside the tank.
    /// @details Original: FishBuyWindow.xml, MK_Menu.pas TFishBuyForm.
    class FishBuyWindow {
        public:
            /// @brief Loads the layout and prepares a fish of each kind to show.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param tank The tank window the form sits beside.
            FishBuyWindow(
                GameServices& services,
                ui::Screen& screen,
                FishTankWindow& tank
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form beside the tank, pulling both onto the screen.
            /// @details Original: ShowFishBuyForm.
            /// @param mode 1 buys a fish, 2 discards one.
            void show(int mode);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

        private:
            /// @brief Asks the server to buy the kind, or discards the picked one from the tank.
            /// @details Original: ConfirmFishBuy.
            void confirm();

            /// @brief Picks the previous kind of fish.
            /// @details Original: PrevFishKind.
            void previousKind();

            /// @brief Picks the next kind of fish.
            /// @details Original: NextFishKind.
            void nextKind();

            /// @brief Paints the picked fish and, when buying, the gold it costs.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top left of the paint area on the canvas.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            FishTankWindow& m_tank;
            ui::Window& m_window;
            const ui::Spot& m_fishSpot;
            const ui::Spot& m_fishOffsetSpot;
            const ui::Spot& m_goldSpot;
            const ui::Spot& m_priceSpot;
            gfx::PictureId m_goldIcon;
            std::array<TankFish, FishTank::kKinds + 1> m_fishes{};
            int m_mode = 1;
            int m_kind = 3;
            int m_index = 0;
    };

}

#endif
