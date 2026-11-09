#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_CONFUSE_STONE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_CONFUSE_STONE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"
#include "protocol/ItemInstance.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <string>

namespace client {

    /// @brief The Chaos Crystals' form (ConfuseStoneWindow.xml): a gear piece and a crystal are dropped on it; Confirm raises the mounted stone by a step.
    /// @details Original: CJ_Menu.pas TConfuseStoneForm; Chaos Crystal 61062, Shadow 61079, Pure 61080. Confirm sends 23/127, the answer 23/214 lights the next of the 16 marks of the wheel.
    class ConfuseStoneWindow {
        public:
            /// @brief Callbacks into the scene: fight state, item window place, mark redraw, packet send and the confirm question.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
            };

            static constexpr int kSteps = 16; // strength steps (stones on the bar), 1-based
            static constexpr int kMark = BagMarks::kConfuseStone;

            /// @brief Loads the window layout and wires its cells, buttons and painters.
            /// @param services Game services (layouts, items, pictures).
            /// @param screen Screen the layout is loaded into.
            /// @param inventory The player's inventory (bag).
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Scene callbacks.
            ConfuseStoneWindow(
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

            /// @brief Opens the form beside the item window and sets the crystal's place (UseGridItem: ShowCtrl, SetStoneSlot).
            /// @param stoneSlot Bag place of the crystal used.
            void open(int stoneSlot);

            /// @brief Closes the form and frees its marks (HideCtrl).
            void close();

            /// @brief Takes a bag item dropped on the form or its place for the gear piece (PutConfuseStoneEquip).
            /// @param bagSlot Bag place of the dropped item.
            void putEquip(int bagSlot);

            /// @brief Takes a bag crystal dropped on the crystal's place (a crystal of the bag only).
            /// @param bagSlot Bag place of the dropped crystal.
            void putStone(int bagSlot);

            /// @brief Handles the server's answer (23/214, RecvConfuseStoneResult): the new step of the stone mounted in the piece of a bag place.
            /// @param step New step of the mounted stone, 1..16.
            /// @param bagSlot Bag place of the piece.
            void result(int step, int bagSlot);

            /// @brief Per-frame update: counts the bag's crystals, sets the buttons and shows the info of the cell under the mouse (DrawControl).
            void update();

        private:
            /// @brief Empties the gear piece place (ClearEquipSlot, the right button).
            void clearEquip();

            /// @brief Empties the crystal place (ClearStoneSlot).
            void clearStone();

            /// @brief Asks the confirm question, then sends (AskConfuseStone).
            void confirm();

            /// @brief Sends the request to raise the stone by a step (23/127, SendConfuseStone).
            void send();

            /// @brief Counts the crystals of the stone's level the bag holds that are free to use.
            /// @return Count, at most 999.
            int crystalCount() const;

            /// @brief Paints what lies under the form's controls (the wheel's marks).
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            void paintBelow(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints an item icon with its count in a cell.
            /// @param canvas Target canvas.
            /// @param at Top-left of the cell.
            /// @param item Item to draw.
            /// @param count Count shown.
            void paintCell(gfx::Canvas& canvas, core::Point at, const protocol::ItemInstance& item, int count);

            /// @brief Removes the form's use mark (27) from a bag place.
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
            ui::Element& m_equipCell;
            ui::Element& m_stoneCell;
            ui::ImageButton& m_confirm;
            const ui::Spot& m_cellSpot;
            const ui::Spot& m_countSpot;
            int m_equipSlot = 0;
            int m_stoneSlot = 0;
            int m_stoneLevel = 0;
            int m_strength = 0;
            bool m_askConfirm = true;
            int m_count = 0;
            protocol::ItemInstance m_stone;
            protocol::ItemInstance m_shownEquip;
            std::array<int, kSteps + 2> m_frames{};
    };

}

#endif
