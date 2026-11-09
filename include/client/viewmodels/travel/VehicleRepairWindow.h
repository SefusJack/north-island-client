#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_VEHICLE_REPAIR_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_VEHICLE_REPAIR_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The garage's vehicle repair form: drop a bag vehicle, confirm the points cost, send the repair request.
    /// @details Original: VehicleRepairWindow.xml, THL_FixTrafficForm; sends 23/95, the server answers 23/81.
    class VehicleRepairWindow {
        public:
            /// @brief Callbacks the window uses to reach the rest of the game.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void()> marksChanged;
                std::function<int()> imPoints;
                std::function<void(const std::string& text, std::function<void()> onOk)> confirm;
            };

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services (layout, items, sounds).
            /// @param screen Screen the window is loaded into.
            /// @param inventory Player inventory (bag items).
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param hooks Callbacks to the rest of the game.
            VehicleRepairWindow(
                GameServices& services,
                ui::Screen& screen,
                Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when the window is shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form and asks the server for the points.
            /// @details Original: 23/81 code 0, ShowCtrl.
            void open();

            /// @brief Hides the form; the vehicle is taken out unless the repair was asked for.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Handles the server's repair answer.
            /// @param code Answer code: 0 open the form, 1 work started, 2 new wear, 3 and 4 refused.
            /// @param value New used amount (code 2).
            /// @details Original: 23/81, RecvVehicleRepairResult.
            void result(int code, int value);

            /// @brief Puts a bag vehicle into the repair space.
            /// @param bagSlot Bag slot of the dropped item.
            /// @details Original: PutVehicleForRepair.
            void put(int bagSlot);

        private:
            /// @brief Draws the points cost and the vehicle in its space.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Takes the vehicle out of the space (right button).
            /// @details Original: RemoveVehicleForRepair.
            void take();

            /// @brief Clears the repair slot and its bag marks.
            /// @details Original: ClearVehicleRepairSlot.
            void clear();

            /// @brief Checks the vehicle and asks the player to confirm the point cost.
            /// @details Original: ConfirmVehicleRepair.
            void confirm();

            /// @brief Checks points and sends the repair request.
            /// @details Original: SendVehicleRepair.
            void send();

            /// @brief Runs the checks made before the points are asked for.
            /// @return True when the vehicle may be repaired.
            bool checked();

            GameServices& m_services;
            ui::Screen& m_screen;
            Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_costSpot;
            int m_slot = 0;
            bool m_requestSent = false;
            std::string m_cost;
    };

}

#endif
