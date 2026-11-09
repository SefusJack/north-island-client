#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_REPAIR_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_REPAIR_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "protocol/ItemInstance.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The Repairing Spanner / Repairing Potion form: repairs a piece of worn gear.
    /// @details Original: RepairWindow.xml, MZ_Menu.pas TMZ_RepairForm, items of sub type 130. A piece of worn gear of the bag is dropped on the place, OK sends 23/84 and says "<name> Repaired". The tool's place and the gear's are in use (mark 11) while the form is up.
    class RepairWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFight;
            };

            static constexpr int kRepairSubType = 130; // item sub type 130: repair tool

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param inventory The player's inventory.
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            RepairWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form when the tool of a bag place was used.
            /// @details Original: OpenRepairForm.
            /// @param toolSlot Bag slot of the tool.
            void open(int toolSlot);

            /// @brief Takes a bag item dropped on the form.
            /// @details Original: PutRepairItem.
            /// @param bagSlot Bag slot of the dropped item.
            void put(int bagSlot);

            /// @brief Shows the placed gear in the cell and its info under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            /// @brief Hides the form and frees the places.
            /// @details Original: HideCtrl and CloseRepairForm.
            void close();

            /// @brief Lets the gear go from the place (the right button on the place).
            /// @details Original: RemoveRepairItem.
            void removeItem();

            /// @brief Sends the repair request.
            /// @details Original: ConfirmRepair.
            void confirm();

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_cell;
            int m_toolSlot = 0;
            int m_slot = 0;
            int m_itemId = 0;
    };

}

#endif
