#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_PACKING_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_PACKING_WINDOW_H

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

    /// @brief The Present Case form: packs a piece of item-mall gear into an empty case.
    /// @details Original: PackingWindow.xml, HL_Menu.pas TPackingForm, items 34200 and 34194. OK asks "Use <case>?" and sends 23/112 option 1; a packed case asks "Items can't be traded if you open the box / Sure?" and sends option 2. The answer (23/107, 23/205) says "Packing Finished".
    class PackingWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, std::function<void()>)> ask;
            };

            static constexpr int kMark = BagMarks::kPacking;
            static constexpr int kEmptyItem = 34200; // item 34200: Present Case (empty, type 25)
            static constexpr int kPackedItem = 34194; // item 34194: Present Case (packed, type 25)

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param inventory The player's inventory.
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            PackingWindow(
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

            /// @brief Handles a case of a bag place being used: opens the form or asks to open a packed case.
            /// @details Original: OpenPackingForm.
            /// @param boxSlot Bag slot of the case.
            void open(int boxSlot);

            /// @brief Takes a bag item dropped on the form's place.
            /// @details Original: PutPackingItem.
            /// @param bagSlot Bag slot of the dropped item.
            void put(int bagSlot);

            /// @brief Handles the server's packing answer (code 1: the piece is packed).
            /// @details Original: RecvPackingResult.
            /// @param code Result code.
            void result(int code);

            /// @brief Checks the case and piece still exist and shows the piece's info under the mouse.
            /// @details Original: DrawControl: CheckPackingItems.
            void update();

        private:
            /// @brief Hides the form and empties the places when no request waits.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Resets the form and frees the cells.
            /// @details Original: ResetPackingForm.
            void reset();

            /// @brief Removes the piece from the place and frees its cell (nothing while a request waits).
            /// @details Original: ClearPackingImage.
            void clearImage();

            /// @brief Asks "Use <case>?" before sending.
            /// @details Original: ConfirmPacking.
            void confirm();

            /// @brief Sends the packing request and closes the form, keeping the cells marked.
            /// @details Original: SendPacking.
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
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Element& m_image;
            const ui::Spot& m_countSpot;
            int m_boxSlot = 0;
            int m_bagSlot = 0;
            bool m_requestSent = false;
            protocol::ItemInstance m_shown;
    };

}

#endif
