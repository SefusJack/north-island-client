#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_TRANSPORT_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_TRANSPORT_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/ItemInstance.h"
#include "ui/DrawingArea.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>
#include <optional>

namespace client {

    /// @brief Window listing the places a ridden transport vehicle goes to; a double click on a place sets off if the fuel is enough.
    /// @details Original: TransportWindow.xml, WS_Menu.pas TWS_TransmiForm. Kinds: 1 village robot, 2 submarine, 3 landing craft;
    /// setting off plays a light round the player, then 69 and the map change.
    class TransportWindow {
        public:
            /// @brief Callbacks into player and scene state the window needs.
            struct Hooks {
                std::function<int()> vehicleItem;
                std::function<std::optional<protocol::ItemInstance>()> vehicleBagItem;
                std::function<int()> sceneType;
                std::function<void(int place)> setOff;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param messages Player message line.
            /// @param hooks Callbacks into player and scene state.
            TransportWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Lists the places of a kind of vehicle and shows the window.
            /// @details Original: FVehicleKind and ShowCtrl.
            /// @param kind Vehicle kind (1 village robot, 2 submarine, 3 landing craft).
            void open(int kind);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Gives the vehicle kind of the listed places.
            /// @return The kind last opened.
            int kind() const { return m_kind; }

        private:
            /// @brief Sets off to a place if the fuel is enough, then closes.
            /// @details Original: PickTeleportTarget (a double click on a place).
            /// @param index Place index counted from 0.
            void pick(int index);

            /// @brief Paints the submarine or landing craft picture over the title.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ListBox& m_places;
            const ui::Spot& m_pictureSpot;
            gfx::PictureId m_submarine;
            gfx::PictureId m_landingCraft;
            int m_kind = 0;
    };

}

#endif
