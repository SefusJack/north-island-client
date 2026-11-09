#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_REINFORCE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_REINFORCE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"
#include "protocol/ItemInstance.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The fusion scrolls' form: raises the fusion level of a piece of gear.
    /// @details Original: ReinforceWindow.xml, HL_Menu.pas TReinforceForm; the scrolls of IsReinforceItem. A piece of gear of the bag is dropped on its place, OK asks "Use <scroll>?" and sends 23/121; the answer (23/210) says how the fusion level went.
    class ReinforceWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, std::function<void()>)> ask;
            };

            static constexpr int kMark = BagMarks::kReinforce;

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param inventory The player's inventory (fusion levels are updated here).
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            ReinforceWindow(
                GameServices& services,
                ui::Screen& screen,
                Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form when a scroll of a bag place was used.
            /// @details Original: OpenReinforceForm.
            /// @param scrollSlot Bag slot of the scroll.
            void open(int scrollSlot);

            /// @brief Takes a bag item dropped on the form's place.
            /// @details Original: PutReinforceItem.
            /// @param bagSlot Bag slot of the dropped item.
            void put(int bagSlot);

            /// @brief Handles the server's answer: code 1 the fusion level of a bag place is now newLevel, code 2 it failed.
            /// @details Original: RecvReinforceResult.
            /// @param code Result code.
            /// @param bagSlot Bag slot of the piece.
            /// @param newLevel New fusion level.
            void result(int code, int bagSlot, int newLevel);

            /// @brief Checks the scroll and piece still exist and shows the piece's info under the mouse.
            /// @details Original: DrawControl: CheckReinforceItems.
            void update();

        private:
            /// @brief Hides the form and empties the places when no request waits.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Resets the form and frees the cells.
            /// @details Original: ResetReinforceForm.
            void reset();

            /// @brief Removes the piece from the place and frees its cell (nothing while a request waits).
            /// @details Original: ClearReinforceImage.
            void clearImage();

            /// @brief Asks "Use <scroll>?" before sending.
            /// @details Original: ConfirmReinforce.
            void confirm();

            /// @brief Sends the request and closes the form, keeping the cells marked.
            /// @details Original: SendReinforce.
            void send();

            /// @brief Paints the piece in the place.
            /// @param canvas Target canvas.
            /// @param at Top-left of the image area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Clears this form's use mark from a bag cell.
            /// @param slot Bag slot.
            void release(int slot);

            GameServices& m_services;
            ui::Screen& m_screen;
            Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Element& m_image;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_levelSpot;
            const ui::Spot& m_countSpot;
            int m_scrollSlot = 0;
            int m_bagSlot = 0;
            bool m_requestSent = false;
            protocol::ItemInstance m_shown;
    };

}

#endif
