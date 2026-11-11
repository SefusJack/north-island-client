#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_HOME_ADORN_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_HOME_ADORN_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HousePillars.h"
#include "client/models/world/Camera.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The Handcart's form and the house pillar editing it starts (Add, Remove, Move).
    /// @details Original: HomeAdornWindow.xml, RE_Menu.pas TRe_HomeAdornForm (owner's click on piece 39065), DZ_Thing.pas TPillarManage (HandlePillarClick, HandlePillarDrop, SetPillarEditMode); closing asks "Are you sure you want to change?" when pillars changed and then sends them (65/11).
    class HomeAdornWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<int()> floor;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
                std::function<bool()> locked;
            };

            /// @brief Loads the form and wires its buttons.
            /// @param services Game services (layouts).
            /// @param screen Screen the form is loaded into.
            /// @param pillars Pillar model being edited.
            /// @param hooks Owner callbacks.
            HomeAdornWindow(
                GameServices& services,
                ui::Screen& screen,
                HousePillars& pillars,
                Hooks hooks
            );

            /// @brief Hides the form when it is up, else shows it (adding mode).
            /// @param furnIndex Furniture index of the clicked Handcart.
            /// @details Original: ToggleHomeAdornForm for the Handcart's piece.
            void toggle(int furnIndex);

            /// @brief Handles a press on the floor while pillars are edited.
            /// @param mouse Mouse position on screen.
            /// @param camera Camera used to convert to map coordinates.
            /// @return True when the press was taken.
            /// @details Original: DXDraw1MouseDown, THomePillar.HandleMouseDown.
            bool press(core::Point mouse, const Camera& camera);

            /// @brief Puts a carried pillar down when the button comes up.
            /// @param mouse Mouse position on screen.
            /// @details Original: HandlePillarDrop.
            void release(core::Point mouse);

            /// @brief Lets a carried pillar follow the mouse (the cell its middle is over).
            /// @param mouse Mouse position on screen.
            void drag(core::Point mouse);

            /// @brief Closes the form when the player is out of the house floor room.
            void update();

        private:
            /// @brief Ends pillar editing and asks about changed pillars before hiding.
            /// @details Original: HideCtrl (SetPillarEditMode 0).
            void close();

            /// @brief Hides the form window.
            void hide();

            /// @brief Shows the form and starts adding pillars.
            void show();

            /// @brief Finds the pillar grid cell (1..16, 1..24) under a map point.
            /// @param mapPoint Point in map coordinates.
            /// @return The cell, or (0, 0) outside the grid.
            static core::Point cellAt(core::Point mapPoint);

            /// @brief Finds the cell the middle of the carried pillar is over.
            /// @param mouse Mouse position on screen.
            /// @return The cell, or (0, 0) outside the grid.
            core::Point carriedCell(core::Point mouse) const;

            ui::Screen& m_screen;
            HousePillars& m_pillars;
            Hooks m_hooks;
            ui::Window& m_window;
            int m_furnIndex = 0;
            core::Point m_grabMouse;
    };

}

#endif
