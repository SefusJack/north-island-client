#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_UTENSIL_STALL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_UTENSIL_STALL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/ItemInstance.h"
#include "protocol/OwnStall.h"
#include "protocol/StallSale.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Window of the player's own stall of large items: choose pieces, set prices, open and end the stall, see sales.
    /// @details Original: UtensilStallWindow.xml, TTH_UtensilStallForm (Sell Large Item in the gold menu); Set a Stall 23/65,
    /// End Stall 23/66, what sold 23/134.
    class UtensilStallWindow {
        public:
            /// @brief Callbacks into player and scene state the window needs.
            struct Hooks {
                std::function<int()> mapId;
                std::function<bool()> noStallScene;
                std::function<bool()> inTeam;
                std::function<bool()> keepsUtensilStall;
                std::function<bool()> keepsAnyStall;
                std::function<bool()> riding;
                std::function<void(const std::string& line)> chatLine;
                std::function<bool()> usesVehicle;
                std::function<bool()> fishing;
                std::function<bool()> remoteOn;
                std::function<void()> bagChanged;
            };

            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param inventory Player's bag.
            /// @param marks Bag cell marks (in-use, locks, vehicle state).
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Callbacks into player and scene state.
            UtensilStallWindow(
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

            /// @brief Opens the form for a new stall with the first large bag piece chosen, or the kept stall's; no piece, no form.
            /// @details Original: TCY_MoneyMenu.StallUtensilClick.
            void openFromMenu();

            /// @brief Shows the form with a fresh choice and the places as they are.
            /// @details Original: ShowCtrl, also from TVenderItem.HandleDblClick on the own stall.
            void open();

            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

            /// @brief Locks the prices and hides the form once the stall is open.
            /// @details Original: HandleOpenUtensilStallPacket.
            void stallOpened();

            /// @brief Resets the form after the stall closed.
            /// @details Original: CloseUtensilStall.
            void stallClosed() { reset(); }

            /// @brief Reports a sale in the chat and empties the sold place.
            /// @details Original: RecvUtensilStallSale (23/134).
            /// @param sale Sale record from the server.
            void showSale(const protocol::StallSale& sale);

            /// @brief Describes the piece under the mouse in the item info popup.
            void update();

        private:
            /// @brief One stall place: the bag slot it comes from and the item held.
            struct Slot {
                int bagSlot = 0;
                protocol::ItemInstance item;
            };

            /// @brief Finds the next stallable bag slot after a given one.
            /// @details Original: FindNextStallableBagSlot.
            /// @param from Bag slot to start after (0 for the start).
            /// @return The slot found, or from when none.
            int findNext(int from) const;

            /// @brief Finds the previous stallable bag slot before a given one.
            /// @details Original: FindPrevStallableBagSlot.
            /// @param from Bag slot to start before (0 for the end).
            /// @return The slot found, or from when none.
            int findPrevious(int from) const;

            /// @brief Tells whether a bag piece can be put in a stall of large items.
            /// @param bagSlot Bag slot to test.
            /// @return True for a multi-cell piece or vehicle that is tradable, unlocked and free.
            bool stallable(int bagSlot) const;

            /// @brief Shows the bag piece chosen with the arrows.
            /// @details Original: ShowCurrentStallableItem.
            void showSelected();

            /// @brief Shows the piece a stall place holds, on the marked frame while marked.
            /// @details Original: SetUtensilStallSlot.
            /// @param slot Stall place (1-based).
            void showSlot(int slot);

            /// @brief Puts the chosen piece into the first free place and moves the choice on.
            /// @details Original: AddCurrentUtensilToStall.
            void add();

            /// @brief Chooses the next stallable bag piece.
            /// @details Original: ShowNextStallableItem.
            void selectNext();

            /// @brief Chooses the previous stallable bag piece.
            /// @details Original: ShowPrevStallableItem.
            void selectPrevious();

            /// @brief Marks a stall place or takes its mark off.
            /// @details Original: ToggleUtensilStallSlot.
            /// @param slot Stall place (1-based).
            void toggleSlot(int slot);

            /// @brief Returns the marked piece to the choice; its price stays as typed.
            /// @details Original: RemoveSelectedUtensilStallSlot.
            void removeMarked();

            /// @brief Closes the form, resetting it when no stall is kept.
            /// @details Original: CloseUtensilStallForm.
            void close();

            /// @brief Runs the game's checks, then sends the open-stall request (Set a Stall, 23/65).
            /// @details Original: StartUtensilStall.
            void keep();

            /// @brief Sends the close-stall request (End Stall, 23/66) and resets the form.
            /// @details Original: StopUtensilStall.
            void revoke();

            /// @brief Empties every place so the prices may be typed again.
            /// @details Original: ResetUtensilStall.
            void reset();

            /// @brief Puts a piece's icon with its middle at a point of the form.
            /// @param grid Icon grid to place and fill.
            /// @param cell Icon cell to show.
            /// @param centre Middle point in window coordinates.
            void place(ui::IconGrid& grid, const ui::IconCell& cell, core::Point centre);

            /// @brief Paints the stall buttons' visibility and the memos of the chosen and put-up pieces.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Gives the price box of a stall place.
            /// @param slot Stall place (1-based).
            /// @return The price text box.
            ui::TextBox& price(int slot);

            /// @brief Reads the typed price of a stall place.
            /// @param slot Stall place (1-based).
            /// @return The price, or 0 when empty.
            std::int32_t priceOf(int slot);

            static constexpr int kSlots = 3;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_shopName;
            ui::IconGrid& m_selectGrid;
            const ui::Spot& m_memoSpot;
            const ui::Spot& m_selectedSpot;
            const ui::Spot& m_selectedMemoSpot;
            std::array<Slot, kSlots + 1> m_slots{};
            std::array<bool, kSlots + 1> m_marked{};
            protocol::ItemInstance m_selected;
            int m_selBagSlot = 0;
            int m_count = 0;
            int m_selSlot = 0;
            bool m_open = false;
            std::string m_lastName;
    };

}

#endif
