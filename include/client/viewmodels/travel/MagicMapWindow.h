#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_MAGIC_MAP_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_MAGIC_MAP_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief World map window opened by the teleport items (Teleport Scroll, Bamboo Dragonflies); picking a place asks and teleports.
    /// @details Original: MagicMapWindow.xml, TMagicMap. Teleport Scroll is mode 1, Bamboo Dragonflies mode 2; the item's place is put in
    /// use (mark 16) and forty places are shown; Confirm sends 23/89.
    class MagicMapWindow {
        public:
            /// @brief Callbacks into player, scene and dialog state the window needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inTeam;
                std::function<std::string()> refusal;
                std::function<bool()> inHouse;
                std::function<bool()> cannotLeaveScene;
                std::function<int()> mapId;
                std::function<void(const std::string& text, std::function<void()> onOk)> ask;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param marks Bag cell marks (use mark 16 marks the item's place).
            /// @param messages Player message line.
            /// @param hooks Callbacks into player, scene and dialog state.
            MagicMapWindow(
                GameServices& services,
                ui::Screen& screen,
                BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Runs the checks, marks the item's place in use and shows the map (the caller closes the item window).
            /// @details Original: UseMagicMapItem.
            /// @param bagSlot Bag slot of the teleport item.
            /// @param mode 1 for the Teleport Scroll, 2 for the Bamboo Dragonflies.
            /// @return True when the map was shown.
            bool use(int bagSlot, int mode);

            /// @brief Hides the map and frees the item's place.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Frees a bag place the server released.
            /// @details Original: RecvMagicMapRelease (23/169): the server frees the place.
            /// @param bagSlot Bag slot to free.
            void release(int bagSlot);

        private:
            /// @brief Handles a click on a place: the refusals again, then the teleport question.
            /// @details Original: HandleMagicMapClick.
            /// @param place Place number (1-based).
            void placeClicked(int place);

            /// @brief Sends the teleport with the place, the item's place and the mode, then closes.
            /// @details Original: ConfirmMagicMapTeleport (23/89).
            void confirm();

            ui::Screen& m_screen;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            int m_targetPlace = 0;
            int m_itemSlot = 0;
            int m_mode = 0;
    };

}

#endif
