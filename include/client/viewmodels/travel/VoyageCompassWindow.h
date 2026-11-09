#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_VOYAGE_COMPASS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_VOYAGE_COMPASS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/items/BagMarks.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Point.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The Voyage compass form: shows the Voyage map's picture with the player's figure on it, foldable into a small box.
    /// @details Original: VoyageCompassWindow.xml, HL_Menu.pas THL_WorldMapForm, item 34004; used on Voyage maps 58001 and 58002.
    class VoyageCompassWindow {
        public:
            /// @brief Callbacks the window uses to read game state.
            struct Hooks {
                std::function<int()> mapId;
                std::function<core::Point()> mapSize;
                std::function<core::Point()> playerPosition;
                std::function<const CharacterLook&()> ownLook;
                std::function<int(int)> bagItem;
            };

            static constexpr int kCompassItem = 34004; // item 34004: Compass (item type 25)

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services (layout, pictures, characters).
            /// @param screen Screen the window is loaded into.
            /// @param marks Bag cell marks (lock checks).
            /// @param messages Player message line.
            /// @param hooks Callbacks to read game state.
            VoyageCompassWindow(
                GameServices& services,
                ui::Screen& screen,
                const BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when the window is shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Uses the compass item: opens the form on a Voyage map.
            /// @param bagSlot Bag slot holding the compass.
            /// @details Original: UseCompassItem.
            void use(int bagSlot);

        private:
            /// @brief Hides the form and resets the compass state.
            /// @details Original: HideCtrl and ResetCompassMap.
            void close();

            /// @brief Folds the open form into a small box, or reopens a folded one.
            /// @details Original: ToggleCompassMinimize.
            void toggle();

            /// @brief Closes the form when its item or map is gone, otherwise draws the player's figure on the map.
            /// @param canvas Target canvas.
            /// @details Original: DrawControl and DrawCompassPlayer.
            void paint(gfx::Canvas& canvas);

            GameServices& m_services;
            ui::Screen& m_screen;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ImageButton& m_minimize;
            const ui::Spot& m_placeSpot;
            const ui::Spot& m_borderSpot;
            const ui::Spot& m_foldedSpot;
            const ui::Spot& m_buttonSpot;
            int m_mode = 0;
            int m_slot = 0;
            int m_mapId = 0;
            double m_scaleX = 0;
            double m_scaleY = 0;
            int m_buttonTop = 0;
    };

}

#endif
