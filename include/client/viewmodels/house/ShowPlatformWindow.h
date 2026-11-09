#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_SHOW_PLATFORM_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_SHOW_PLATFORM_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
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

    /// @brief The Show Platform form of a house: one of the vehicles parked in the Garage stands on the platform for the visitors.
    /// @details Original: ShowPlatformWindow.xml, MK_Menu.pas TUseApronForm, item 39001; the platform's click asks the server (62/55) and its answer (62/54) shows the form when a Garage stands in the house; a press on a Garage cell puts its vehicle on show (62/49), the right button on the cell on show takes it down again.
    class ShowPlatformWindow {
        public:
            /// @brief Loads the form and wires its cell events.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the form is loaded into.
            /// @param house The house interior (utensils, car cases).
            /// @param info Item info popup.
            /// @param send Sends a packet to the server.
            ShowPlatformWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                ItemInfoPopup& info,
                std::function<void(const core::Bytes&)> send
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Takes the clicked platform piece as the form's and asks the server (62/55).
            /// @param furnIndex Index of the clicked furniture piece.
            void request(int furnIndex);

            /// @brief Handles the server's answer: stores the restrain state and shows the form when a Garage stands in the house.
            /// @param restrained FRestrainState from the answer.
            /// @param garage Whether a Garage stands in the house.
            /// @details Original: 62/54, FRestrainState, then ShowCtrl for a Show Platform standing in the house.
            void opened(int restrained, bool garage);

            /// @brief Hides the form and clears the restrain state.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Stores the vehicle shown in a cell.
            /// @param stored The stored cupboard item (place and item).
            /// @details Original: 62/47 for a Show Platform's piece (SetCarCaseSlot).
            void parked(const protocol::FurnitureCupboardItem& stored);

            /// @brief Sets the Garage cell whose vehicle is on show.
            /// @param cell Cell 1..4, or 0 for none.
            /// @details Original: FExhibitSlot (set by 23/144 and by a press on a cell).
            void setExhibitSlot(int cell) { m_exhibitSlot = cell; }

            /// @brief Takes the vehicle on show down (62/49) before it goes back into the bag.
            /// @param cell Garage cell being taken.
            /// @details Original: TakeCarCaseItem.
            void garageTaking(int cell);

            /// @brief Closes the form when its piece is gone and shows the info of the vehicle under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            static constexpr int kCells = 4;

            /// @brief Draws the display strip and, for a restrained Garage, the restrain picture.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @details Original: DrawControl.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Puts the vehicle of a cell on show (62/49).
            /// @param cell Cell 1..4.
            /// @details Original: SetExhibitSlotClick.
            void choose(int cell);

            /// @brief Takes the vehicle of a cell down from the platform when it is the one on show.
            /// @param cell Cell 1..4.
            /// @details Original: ClearExhibitSlotClick.
            void release(int cell);

            /// @brief Tells whether the form's piece is a Show Platform standing in the house.
            /// @return True when it is.
            bool pieceIsPlatform() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            ItemInfoPopup& m_info;
            std::function<void(const core::Bytes&)> m_send;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::DrawingArea>, kCells + 1> m_areas;
            const ui::Spot& m_restrainSpot;
            const ui::Spot& m_displaySpot;
            gfx::PictureId m_restrainPicture;
            gfx::PictureId m_displayPicture;
            std::array<protocol::ItemInstance, kCells + 1> m_images{};
            int m_furnIndex = 0;
            int m_restrained = 0;
            int m_exhibitSlot = 0;
            int m_pressedCell = 0;
    };

}

#endif
