#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_PARKING_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_PARKING_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "protocol/ItemInstance.h"
#include "protocol/ParkedVehicle.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <vector>

namespace client {

    /// @brief Parking lot window of the Parking Clerk: four places for vehicles, park from the bag and take back.
    /// @details Original: ParkingWindow.xml, MK_Menu.pas TParkForm. A bag vehicle let go on a place is parked (23/92), the right button
    /// on a parked vehicle takes it back (23/93); the server lists the places (23/153, 23/154), answers (23/155) and opens the form (23/157).
    class ParkingWindow {
        public:
            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory Player's bag.
            /// @param marks Bag cell marks (locks).
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param send Sends a packet to the server.
            ParkingWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                std::function<void(const core::Bytes&)> send
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Handles the server's open answer.
            /// @details Original: 23/157: 1 shows the form, 2 says the lot has no room.
            /// @param code 1 to show the form, 2 for no room.
            void opened(int code);

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Sets every listed parked vehicle.
            /// @details Original: RecvParkList (23/153).
            /// @param cells Parked vehicles of the lot.
            void setList(const std::vector<protocol::ParkedVehicle>& cells);

            /// @brief Sets one parked vehicle.
            /// @details Original: RecvParkSlot (23/154).
            /// @param cell Parked vehicle with its place.
            void setCell(const protocol::ParkedVehicle& cell);

            /// @brief Empties every place.
            /// @details Original: ClearParkSlots (23/156), also when the player logs out.
            void clear();

            /// @brief Shows the result message of a park or redeem request.
            /// @details Original: RecvParkResult (23/155).
            /// @param code Result code.
            /// @param itemId Item id of the vehicle concerned.
            void result(int code, int itemId);

            /// @brief Runs the drop checks and requests parking of a bag vehicle.
            /// @details Original: the drop checks (Se_EquipUI HandleItemDrop for slot kind 19) and RequestParkVehicle (23/92).
            /// @param bagSlot Bag slot of the dropped item.
            /// @param cell Place it was let go on (1..4).
            void put(int bagSlot, int cell);

            /// @brief Shows the info of the vehicle under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            static constexpr int kCells = 4; // number of parking cells (1..4)

            /// @brief Paints the title picture.
            /// @details Original: DrawControl: the title picture.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Finds the first free place.
            /// @details Original: FindFreeParkSlot.
            /// @return The place (1..4), or 0 when none.
            int freeCell() const;

            /// @brief Requests taking a parked vehicle back into the bag (right button).
            /// @details Original: RequestRedeemVehicle (23/93).
            /// @param cell Place (1..4).
            void redeem(int cell);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            std::function<void(const core::Bytes&)> m_send;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::DrawingArea>, kCells + 1> m_areas;
            std::array<protocol::ItemInstance, kCells + 1> m_cells{};
            const ui::Spot& m_titleSpot;
            gfx::PictureId m_title;
            int m_pressedCell = 0;
    };

}

#endif
