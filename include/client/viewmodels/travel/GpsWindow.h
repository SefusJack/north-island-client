#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_GPS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_GPS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Form of the GPS items: shows the recorded places, records the current one and transmits to a recorded one.
    /// @details Original: GpsWindow.xml, WS_Menu.pas TGPSForm, items 34006, 34195 and 34373. Advance GPS has two points, Smart GPS three;
    /// Set Point sends 74/1, Transmit starts the teleport light which sends 74/2 when it has played; a Memory Crystal 37257 raises a GPS
    /// one step (AskGpsItemUpgrade, 74/4).
    class GpsWindow {
        public:
            /// @brief A refusal the scene reports: its text, how long it shows and whether the form closes with it.
            struct Refusal {
                std::string text;
                int ms = 0;
                bool closes = false;
            };

            /// @brief Callbacks into player, scene and dialog state the form needs.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<std::optional<Refusal>(bool teleport)> refusal;
                std::function<void()> closeItemWindow;
                std::function<void()> startTeleport;
                std::function<bool()> fighting;
                std::function<void(const std::string&, std::function<void()>)> askMain;
                std::function<void(const std::string&, std::function<void()>)> askTimed;
                std::function<void(const std::string&, const std::vector<std::string>&, std::function<void(int)>)> choose;
                std::function<std::optional<std::string>(int mapId)> sceneName;
            };

            static constexpr int kPoints = 3; // number of saved location points (1..3)

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory Player's bag.
            /// @param marks Bag cell marks (locks).
            /// @param messages Player message line.
            /// @param hooks Callbacks into player, scene and dialog state.
            GpsWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form beside the item window for the GPS of a bag place.
            /// @details Original: ShowGpsFormForItem.
            /// @param bagSlot Bag slot of the GPS item.
            void open(int bagSlot);

            /// @brief Asks which GPS to raise with the Memory Crystal of a bag place.
            /// @details Original: AskGpsItemUpgrade (74/4).
            /// @param crystalSlot Bag slot of the Memory Crystal.
            void askUpgrade(int crystalSlot);

            /// @brief Stores a recorded place of a point.
            /// @details Original: RecvGpsMarkSlot (74/1).
            /// @param point Point number (1-based).
            /// @param map Map id of the place.
            /// @param x Tile x.
            /// @param y Tile y.
            void setMark(int point, int map, int x, int y);

            /// @brief Builds the teleport request for the place and point Transmit was pressed on.
            /// @details Original: FRequest, 74/2 (the teleport light asks for it when it has played).
            /// @return The request packet.
            core::Bytes teleportRequest() const;

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

        private:
            /// @brief A recorded place: map and tile coordinates.
            struct Mark {
                int map = 0;
                int x = 0;
                int y = 0;
            };

            /// @brief Handles Set Point: checks refusals, asks, then records the place.
            /// @details Original: SetGpsPointClick, SendGpsMark (74/1).
            void setPointClicked();

            /// @brief Handles Transmit: checks refusals, then starts the teleport light and closes the forms.
            /// @details Original: GpsTeleportClick (74/2 after the light).
            void teleportClicked();

            /// @brief Steps to the previous point.
            /// @details Original: PrevGpsPoint.
            void previous();

            /// @brief Steps to the next point.
            /// @details Original: NextGpsPoint.
            void next();

            /// @brief Shows or hides the arrows for the current point and GPS kind.
            /// @details Original: UpdateGpsPageButtons.
            void updateButtons();

            /// @brief Gives the number of points of the GPS in the bag place.
            /// @return 2 for the Advance GPS, 3 for the Smart GPS, else 1.
            int pageCount() const;

            /// @brief Raises the first free GPS of the chosen kind after a timed question.
            /// @details Original: GpsUpgradeChoice, SendGpsItemUpgrade (74/4).
            /// @param choice 0 for Normal to Advance, 1 for Advance to Smart.
            void upgradeChosen(int choice);

            /// @brief Tests a Memory Crystal's bag place.
            /// @details Original: the tests of AskGpsItemUpgrade and GpsUpgradeChoice.
            /// @param slot Bag slot of the crystal.
            /// @return True when the crystal may be used.
            bool crystalUsable(int slot) const;

            /// @brief Looks up an item's name.
            /// @param itemId Item id.
            /// @return The name, or empty when unknown.
            std::string itemName(int itemId) const;

            /// @brief Paints the place's name and coordinates, or the empty point's line.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ImageButton& m_previous;
            ui::ImageButton& m_next;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_xSpot;
            const ui::Spot& m_ySpot;
            const ui::Spot& m_emptySpot;
            std::array<Mark, kPoints> m_points{};
            int m_slot = 0;
            int m_point = 1;
            int m_requestSlot = 0;
            int m_requestPoint = 1;
            int m_crystalSlot = 0;
    };

}

#endif
