#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_GAS_STORAGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_GAS_STORAGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief Gas storage form of a house, serving both the Big and the Small tank.
    /// @details Original: GasCanWindow.xml and SmallGasWindow.xml, CJ_Menu.pas TGasCanForm and TSmallGasForm, furniture 39122 and 39123; Gas items (47009) are marked in the bag and sent to the tank with 62/62 (Big) or 62/63 (Small); the server answers 64/12 or 64/14 with the fuel and 64/13 or 64/15 when it refuses.
    class GasStorageWindow {
        public:
            /// @brief Which of the two tank forms this window is.
            enum class Tank { Big, Small };

            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void()> marksChanged;
                std::function<void()> closeMakeInfo;
            };

            static constexpr int kBigTankItem = 39122; // item 39122: Big Gas Storage (house furniture, type 30)
            static constexpr int kSmallTankItem = 39123; // item 39123: Gas Storage (house furniture, type 30)

            /// @brief Loads the form of the given tank and wires its events.
            /// @param tank Which tank form to build.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the form is loaded into.
            /// @param inventory The bag whose cells are shown.
            /// @param marks Bag cell marks shared with the other windows.
            /// @param messages Sink for player notices.
            /// @param hooks Owner callbacks.
            GasStorageWindow(
                Tank tank,
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form, lets go of its marks and closes the make-thing info form.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the form and lets go of its marks.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Stores the tank's fuel amount.
            /// @param fuel Current fuel (Player.FGasCanFuel or FSmallGasFuel).
            void setFuel(int fuel) { m_fuel = fuel; }

            /// @brief Clears all marks of this form (also when the server refused the deposit).
            /// @details Original: UnmarkAllGasItems and UnmarkAllSmallGasItems.
            void unmarkAll();

            /// @brief Refreshes the bag cells shown in the form each frame.
            /// @details Original: RefreshGridItems.
            void update();

        private:
            static constexpr int kCells = BagMarks::kSlots;

            /// @brief Marks a Gas item of the bag for deposit.
            /// @param slot Bag slot (1-based).
            /// @details Original: MarkGasItem.
            void mark(int slot);

            /// @brief Takes the deposit mark off a bag cell.
            /// @param slot Bag slot (1-based).
            /// @details Original: UnmarkGasItem.
            void unmark(int slot);

            /// @brief Sends the marked cells to the tank unless it is full.
            /// @details Original: FillGasCan and FillSmallGasCan (62/62 or 62/63).
            void fill();

            /// @brief Draws the fuel text and the filled part of the tank picture.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @details Original: DrawControl.
            void paint(gfx::Canvas& canvas, core::Point at);

            Tank m_tank;
            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::IconGrid& m_cells;
            const ui::Spot& m_fuelSpot;
            const ui::Spot& m_slashSpot;
            const ui::Spot& m_capacitySpot;
            std::array<bool, kCells + 1> m_marked{};
            int m_fuel = 0;
    };

}

#endif
