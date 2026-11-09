#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_RETAIN_FUR_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_RETAIN_FUR_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HomeFurniture.h"
#include "client/viewmodels/hud/MainButtons.h"
#include "core/Point.h"
#include "ui/BlockList.h"
#include "ui/ScrollBar.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief The furniture pick-up list: the pieces waiting in the owner's pick-up area, one row each; a row dragged onto the house floor sets the piece down there.
    /// @details Original: RetainFurWindow.xml, RE_Menu.pas TRe_RetainFurForm; its "Pick up the furniture" button (RE_MainBtnForm.FButtons[2]) shows while the owner is in a floor room of the house and the area is not empty.
    class RetainFurWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<bool()> ownHouse;
                std::function<bool()> inFloorRoom;
                std::function<bool()> inOtherRoom;
                std::function<void(const std::string&, int)> notice;
                std::function<void(int)> take;
                std::function<void()> carry;
                std::function<void()> drop;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Game services (layouts, items).
            /// @param screen Screen the form is loaded into.
            /// @param buttons Main buttons, for the pick-up button.
            /// @param furniture The home furniture pieces.
            /// @param hooks Owner callbacks.
            RetainFurWindow(
                GameServices& services,
                ui::Screen& screen,
                MainButtons& buttons,
                const HomeFurniture& furniture,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Closes the list, or opens it in a floor room (says it cannot be used in the house's other rooms).
            /// @details Original: ToggleRetainFurForm, from the button of RE_MainBtnForm.
            void toggle();

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Refreshes the list of retained pieces and the pick-up button (once a frame).
            /// @details Original: LoadRetainedFurniture, and the button's showing and hiding that follow every message about the area.
            void update();

        private:
            /// @brief Rebuilds the list rows, one for each piece whose item is known.
            /// @details Original: FillRetainedFurnitureList (icon in a 34 x 34 area, name beside).
            void fill();

            /// @brief Scrolls the list to a top row and syncs the scroll bar.
            /// @param top Wanted top row index (clamped).
            void scrollTo(int top);

            /// @brief Starts carrying the piece of a pressed row; outside the own house the list closes.
            /// @param row Row index in the list.
            /// @details Original: DragRetainedFurniture.
            void press(int row);

            GameServices& m_services;
            ui::Screen& m_screen;
            MainButtons& m_buttons;
            const HomeFurniture& m_furniture;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_list;
            ui::ScrollBar& m_scroll;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_nameSpot;
            std::vector<int> m_pieces;
    };

}

#endif
