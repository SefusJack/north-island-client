#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_SEWING_KIT_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_SEWING_KIT_WINDOW_H

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

#include <array>
#include <functional>
#include <string>

namespace client {

    /// @brief The Sewing Tools / Weapon Model form: gives one piece of gear another's look.
    /// @details Original: SewingKitWindow.xml, HL_Menu.pas THL_SewingKitForm, items 34167 and 34170. Two pieces of gear of the same kind are dropped on the first two places, the second keeping its attributes and the first (with a question: it disappears) giving its look; the third place shows the result. Confirm asks and sends 23/105 option 1; the answer (23/105 1 or 2) says whether the pose was designed.
    class SewingKitWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, std::function<void()>)> ask;
                std::function<int()> bodyType;
            };

            static constexpr int kMark = BagMarks::kSewingKit;
            static constexpr int kSewingTools = 34167; // item 34167: Sewing Tools
            static constexpr int kWeaponModel = 34170; // item 34170: Weapon Model

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param inventory The player's inventory (the look of a piece is updated here).
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            SewingKitWindow(
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

            /// @brief Opens the form when the kit of a bag place was used.
            /// @details Original: OpenSewingKit.
            /// @param kitSlot Bag slot of the kit.
            void open(int kitSlot);

            /// @brief Takes a bag item dropped on place 1 (the look) or 2 (the attributes) of the form.
            /// @details Original: PutSewingKitItem.
            /// @param bagSlot Bag slot of the dropped item.
            /// @param place Form place, 1 or 2.
            void put(int bagSlot, int place);

            /// @brief Handles the server's answer: code 1 the piece of a bag place now takes this look, code 2 failed.
            /// @details Original: RecvSewingKitResult 1 and 2.
            /// @param code Result code.
            /// @param bagSlot Bag slot of the piece.
            /// @param look Item id whose look the piece takes.
            void result(int code, int bagSlot, int look);

            /// @brief Checks the kit and pieces still exist and shows a place's info under the mouse.
            /// @details Original: DrawControl: CheckSewingKitItems.
            void update();

        private:
            /// @brief Hides the form and empties the places when no request waits.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Resets the form, emptying every place and freeing the kit.
            /// @details Original: ResetSewingKit.
            void reset();

            /// @brief Empties a place, frees its cell and the result place with it.
            /// @details Original: ClearSewingKitSlot.
            /// @param place Form place, 1 to 3.
            void clearPlace(int place);

            /// @brief Fills the places again from the cells they stand for.
            /// @details Original: RefreshSewingKitSlots.
            void refreshPlaces();

            /// @brief Checks the refusals, then puts a piece in a place (or asks first).
            /// @details Original: SetSewingKitSlot.
            /// @param bagSlot Bag slot of the piece.
            /// @param place Form place, 1 or 2.
            /// @param confirm True to ask a question before taking the piece.
            void setPlace(int bagSlot, int place, bool confirm);

            /// @brief Gives the item a place shows (place 3 is the result).
            /// @param place Form place, 1 to 3.
            /// @return The item shown.
            const protocol::ItemInstance& shown(int place) const { return m_images[static_cast<std::size_t>(place - 1)]; }

            /// @brief Asks to confirm before sending.
            /// @details Original: ConfirmSewingKit.
            void confirm();

            /// @brief Sends the request (the places stay as they are until the answer).
            /// @details Original: SendSewingKit.
            void send();

            /// @brief Paints a place's piece icon and count.
            /// @param canvas Target canvas.
            /// @param at Top-left of the place.
            /// @param place Form place, 1 to 3.
            void paint(gfx::Canvas& canvas, core::Point at, int place);

            /// @brief Clears this form's use mark from a bag cell.
            /// @param slot Bag slot.
            void release(int slot);

            /// @brief Shows the refusal "The item can't be used " and optionally the kit's name.
            /// @param withName True to append the kit's name.
            void cannotUse(bool withName);

            /// @brief Gives the name of the kit in use.
            /// @return The kit's item name, empty when unknown.
            std::string kitName() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::Element>, 3> m_cells;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_countSpot;
            int m_kitSlot = 0;
            int m_kitKind = 0;
            int m_poseSlot = 0;
            int m_equipSlot = 0;
            int m_pendingSlot = 0;
            bool m_sending = false;
            std::array<protocol::ItemInstance, 3> m_images;
            int m_previewIcon = 0;
    };

}

#endif
