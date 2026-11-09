#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_CAR_CASE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_CAR_CASE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/house/ShowPlatformWindow.h"
#include "core/Bytes.h"
#include "protocol/FurnitureCupboardItem.h"
#include "protocol/ItemInstance.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief The Garage window of a house (CarCaseWindow.xml, TUseCarCaseForm, the item 39069): four cells for vehicles.
    /// @details Original: WS_Menu.pas TUseCarCaseForm. A bag vehicle let go on a cell is parked (62/47), the right button on a parked vehicle takes it back (62/48); the Garage's click asks the server (62/51) and its answer shows the form, with the cells 3 and 4 covered when the Garage is restrained (they only give back).
    class CarCaseWindow {
        public:
            /// @brief Loads the window layout and wires the cells.
            /// @param services Game services (layouts, pictures, item records).
            /// @param screen Screen the window is loaded into.
            /// @param house The house interior with its utensils and garages.
            /// @param inventory The player's bag.
            /// @param marks Bag slot marks (locks).
            /// @param messages Player message line for refusals.
            /// @param info Item info popup shown under the mouse.
            /// @param platform The platform window told when a vehicle is taken.
            /// @param send Sends a packet to the server.
            CarCaseWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                ShowPlatformWindow& platform,
                std::function<void(const core::Bytes&)> send
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Starts opening a Garage: its piece becomes the form's and the server is asked (62/51).
            /// @details Original: the click on a Garage; FFurnIndex is set and 62/51 sent.
            /// @param furnIndex Furniture piece index of the Garage.
            void request(int furnIndex);

            /// @brief Shows the form with the Garage's restrained state (answer 62/51), only for a Garage standing in the house.
            /// @details Original: 62/51: FRestrainState, then ShowCtrl.
            /// @param restrained Restrained state (1 cells 3 and 4 covered).
            void shown(int restrained);

            /// @brief Changes the restrained state while the form shows (23/136 kinds 8 and 9).
            /// @details Original: FRestrainState; 2 gives back only, 1 as 62/51's.
            /// @param state New restrained state.
            void restrain(int state) {
                if (isOpen()) {
                    m_restrained = state;
                }
            }

            /// @brief Hides the form and clears the restrained state.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Shows a parked vehicle in its cell (62/47); only when the form's own piece is a Garage.
            /// @details Original: SetUseCarCaseSlot reads the piece the form was opened for, not the one the packet names.
            /// @param stored The stored vehicle and its place.
            void parked(const protocol::FurnitureCupboardItem& stored);

            /// @brief Empties a cell after its vehicle was taken (62/48).
            /// @param cell Cell number (1..4).
            void left(int cell);

            /// @brief Handles a bag item let go on a cell: runs the checks of the drop in order, then sends the park request.
            /// @details Original: HandleItemDrop for the slot kind 16 and DropItemOnCarCase.
            /// @param bagSlot Bag slot of the dropped item.
            /// @param cell Cell number (1..4).
            void put(int bagSlot, int cell);

            /// @brief Per frame: closes the form when its piece is gone and shows the info of the vehicle under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            static constexpr int kCells = 4;

            /// @brief Paints the picture over the cells 3 and 4 of a restrained Garage.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Takes the vehicle of a cell back into the bag (right button on a cell); refuses when the bag is full.
            /// @details Original: TakeCarCaseItem.
            /// @param cell Cell number (1..4).
            void take(int cell);

            /// @brief Tells whether the form's piece is a Garage standing in the house.
            /// @return True when it is a Garage.
            bool pieceIsGarage() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            ShowPlatformWindow& m_platform;
            std::function<void(const core::Bytes&)> m_send;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::DrawingArea>, kCells + 1> m_areas;
            const ui::Spot& m_restrainSpot;
            gfx::PictureId m_restrainPicture;
            std::array<protocol::ItemInstance, kCells + 1> m_images{};
            int m_furnIndex = 0;
            int m_restrained = 0;
            int m_pressedCell = 0;
    };

}

#endif
