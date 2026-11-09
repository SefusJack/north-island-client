#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_FISH_FISH_TANK_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_FISH_FISH_TANK_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/fish/FishTank.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The fish tank form: fish swim in it, a click drops fish food, the owner buys and discards fish.
    /// @details Original: FishTankWindow.xml, MK_Menu.pas TFishBoxForm; the owner's 62/56 is answered by 23/160 11, which opens it.
    class FishTankWindow {
        public:
            /// @brief Callbacks into the rest of the game that the form needs.
            struct Hooks {
                std::function<bool()> ownerHere;
                std::function<void()> instructions;
                std::function<void(int mode)> buyForm;
            };

            /// @brief Loads the layout, centres the form and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Sink for short player messages.
            /// @param hooks Callbacks into the game.
            FishTankWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form in front.
            /// @details Original: ShowCtrl.
            void show();

            /// @brief Shows the form after a visitor's click on the tank, without old food.
            /// @details Original: ClearAllFood, ShowCtrl.
            void showToVisitor();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

            /// @brief Handles the tank packet 23/160.
            /// @details Original: RecvFishBoxPacket (DZ_Connect.pas 23/160).
            /// @param payload The whole message.
            void receive(const core::Bytes& payload);

            /// @brief Moves the fish, grains and bubbles on and shows the owner's buttons.
            /// @details Original: RenderWithChildren.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Sends a tank request (23/98) with a kind; the server keeps the kind for later requests that bring none.
            /// @details Original: FStrLists.B[24] holds the kind.
            /// @param request Request code.
            /// @param kind Kind of fish.
            void requestKind(int request, int kind);

            /// @brief Gives the fish tank model.
            /// @return The tank.
            FishTank& tank() { return m_tank; }

            /// @brief Gives the form's place, for the buy form to lie beside it.
            /// @return The form's bounds.
            core::Rect bounds() const { return m_window.bounds(); }

            /// @brief Shifts the form sideways to keep it and the buy form on the screen.
            /// @param dx Pixels to move right.
            void moveBy(int dx) { m_window.setPosition({m_window.bounds().x + dx, m_window.bounds().y}); }

        private:
            /// @brief Asks for a grain of fish food where the server allows it.
            /// @details Original: HandleMouseDown.
            /// @param local Click point inside the form.
            void mouseDown(core::Point local);

            /// @brief Asks the server to open the buy form.
            /// @details Original: RequestBuyFish.
            void requestBuy();

            /// @brief Asks the server to open the discard form.
            /// @details Original: RequestDiscardFish.
            void requestDiscard();

            /// @brief Paints the fish, grains and bubbles, and for the owner the fish food and the coin count.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top left of the paint area on the canvas.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_foodIconSpot;
            const ui::Spot& m_foodCountSpot;
            const ui::Spot& m_coinIconSpot;
            const ui::Spot& m_coinCountSpot;
            FishTank m_tank;
            gfx::PictureId m_coinIcon;
            gfx::PictureId m_foodIcon;
            bool m_canFeed = false;
            int m_food = 0;
            int m_count = 0;
            int m_kind = 0;
            int m_dropX = 0;
    };

}

#endif
