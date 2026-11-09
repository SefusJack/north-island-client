#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_CAPSULE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRAVEL_CAPSULE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "protocol/ItemInstance.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief Form of the Space Capsule: drop a bag vehicle on the place and store it as its capsule.
    /// @details Original: CapsuleWindow.xml, MK_Menu.pas TCapsuleForm, item 34270; OK sends 23/125 and uses up the Space Capsule.
    class CapsuleWindow {
        public:
            /// @brief Callbacks into player state and the item window the form needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> fighting;
                std::function<core::Rect()> itemWindowBounds;
                std::function<void(int left)> moveItemWindow;
                std::function<void()> marksChanged;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory Player's bag.
            /// @param marks Bag cell marks (use mark 26 marks the cells in the form).
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Callbacks into player state and the item window.
            CapsuleWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form beside the item window after a Space Capsule was used.
            /// @details Original: OpenCapsuleForm.
            /// @param bagSlot Bag slot of the Space Capsule.
            void open(int bagSlot);

            /// @brief Empties the place, frees both cells and hides the form.
            /// @details Original: CloseCapsuleForm and HideCtrl.
            void close();

            /// @brief Puts a dropped bag vehicle into the place after the drop checks.
            /// @details Original: PutCapsuleItem.
            /// @param bagSlot Bag slot of the dropped item.
            void put(int bagSlot);

            /// @brief Shows the info of the vehicle under the mouse.
            /// @details Original: DrawControl.
            void update();

        private:
            /// @brief Paints the vehicle in the place.
            /// @details Original: DrawControl of the vehicle in the place: its rails read bag place 0 of the game (the image's FCtrlTag, which nothing sets), so the wear rail is always full and the fuel rail is not drawn.
            /// @param canvas Target canvas.
            /// @param at Place origin on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Sends the capsule request for the two places and closes the form.
            /// @details Original: ConfirmCapsule (SendServerMsg 23/125).
            void confirm();

            /// @brief Takes the vehicle out of the place (right button).
            /// @details Original: RemoveCapsuleItem.
            void take();

            /// @brief Clears the shown vehicle image.
            /// @details Original: ClearCapsuleImage.
            void clearImage();

            /// @brief Removes the use mark 26 from a bag place.
            /// @param slot Bag slot (1-based).
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
            int m_capsuleSlot = 0;
            int m_itemSlot = 0;
            protocol::ItemInstance m_shown;
            bool m_pressed = false;
    };

}

#endif
