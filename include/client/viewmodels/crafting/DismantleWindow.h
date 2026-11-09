#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_DISMANTLE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_DISMANTLE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Rect.h"
#include "protocol/ItemInstance.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Tear Pliers' form (DismantleWindow.xml): a gear piece with a mounted stone is dropped on the place; OK asks the server to take the stone out.
    /// @details Original: MK_Menu.pas TMK_plierForm, items 34152 and 34155. OK sends 23/86, the server answers with the piece's new wear (23/143).
    class DismantleWindow {
        public:
            /// @brief Callbacks into the scene: fight state, item window place and mark redraw.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
            };

            static constexpr int kMark = BagMarks::kDismantle;
            static constexpr int kTearPlier = 1;
            static constexpr int kSmallTearPlier = 2;

            /// @brief Loads the window layout and wires its place, buttons and painter.
            /// @param services Game services (layouts, items, pictures, connection).
            /// @param screen Screen the layout is loaded into.
            /// @param inventory The player's inventory (bag).
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Scene callbacks.
            DismantleWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Whether the window is showing.
            /// @return True while visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for the pliers of a bag place (OpenDismantleNearEquip and the mode the caller sets after it).
            /// @param pliersSlot Bag place of the pliers used.
            /// @param mode kTearPlier or kSmallTearPlier.
            void open(int pliersSlot, int mode);

            /// @brief Closes the form and frees its marks (CloseDismantleForm and HideCtrl).
            void close();

            /// @brief Takes a bag item dropped on the place (PutItemForDismantle).
            /// @param bagSlot Bag place of the dropped item.
            void put(int bagSlot);

            /// @brief Handles the server's answer (23/143, RecvDismantleResult): the piece of a bag place has lost its stone.
            /// @param bagSlot Bag place of the piece.
            void result(int bagSlot);

            /// @brief Per-frame update: the piece in the place shows its info (DrawControl).
            void update();

        private:
            /// @brief Paints the piece's icon in the place.
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Asks the server to take the stone out (23/86, ConfirmDismantle).
            void confirm();

            /// @brief Empties the place (ClearDismantleSelection, the right button; ClearDismantleItemImage).
            void clearSelection();

            /// @brief Removes the form's use mark (13) from a bag place.
            /// @param slot Bag place.
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
            int m_pieceSlot = 0;
            int m_pliersSlot = 0;
            int m_mode = 0;
            protocol::ItemInstance m_shown;
    };

}

#endif
