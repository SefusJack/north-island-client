#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_VENDING_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_VENDING_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/house/StallInFurItem.h"
#include "ui/BlockList.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The vending machine form: its five places with what lies in them.
    /// @details Original: VendingWindow.xml, TH_Menu.pas TTH_VendingmachineForm; a buyer clicks a place to buy it, the owner right-clicks one to stop selling it and drops items of the bag on the form to put them in.
    class VendingWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<bool()> inFloorRoom;
                std::function<std::int32_t()> ownId;
                std::function<void(const StallInFurItem&)> buy;
                std::function<void(int)> askStop;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Game services (layouts).
            /// @param screen Screen the form is loaded into.
            /// @param house The house interior (stall items).
            /// @param hooks Owner callbacks.
            VendingWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for the machine of a piece.
            /// @param furnIndex Index of the furniture piece.
            /// @details Original: ShowCtrl for the machine of that piece.
            void open(int furnIndex);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Reads the piece the form is open for.
            /// @return The furniture index.
            int furnIndex() const { return m_furnIndex; }

            /// @brief Finds the place of the form the mouse is over, as the drop of an item works it out.
            /// @return Place 1..5, or 0 for none.
            int rowUnderMouse() const;

        private:
            /// @brief Rebuilds the rows from what lies in the places.
            /// @details Original: DrawControl.
            void refresh();

            /// @brief Per-frame draw hook of the form.
            void paint();

            /// @brief Finds the item of a place of the machine.
            /// @param row Place 0..4.
            /// @return The item (empty when there is none).
            const StallInFurItem& itemOf(int row) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_list;
            const ui::Spot& m_placeSpot;
            const ui::Spot& m_textSpot;
            int m_furnIndex = 0;
    };

}

#endif
