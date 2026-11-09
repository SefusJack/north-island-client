#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_WEAPON_BANK_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_WEAPON_BANK_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/Inventory.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace client {

    /// @brief The guild weapon bank window (GuildWeaponBankWindow.xml, TSe_ArmyWeaponBank): deposit (39/52) and withdraw (39/53) weapon materials.
    /// @details Original: opened by the army form's weapon bank button (WeaponBankClick); the 2014 client never shows that form, so nothing opens it yet.
    class GuildWeaponBankWindow {
        public:
            static constexpr int kKinds = 7;

            /// @brief Loads the window layout and wires its boxes and arrows.
            /// @details Original: weaponCounts is TCY_OrganManage.FWeaponCounts (1..7, filled by 39/27).
            /// @param services Game services (layouts, connection).
            /// @param screen Screen the window is loaded into.
            /// @param bag The player's bag, whose weapon materials (item type 53) can be chosen.
            /// @param weaponCounts Stored amount per weapon kind, indexed 1..7.
            GuildWeaponBankWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& bag,
                const std::array<std::int32_t, kKinds + 1>& weaponCounts
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows or hides the window.
            /// @details Original: ToggleCtrlShown.
            void toggle();

            /// @brief Hides the window.
            void hide() { m_window.setVisible(false); }

            /// @brief Rebuilds the bag cells after the bag changed.
            /// @details Original: UpdateWeaponBankHints.
            void refresh();

        private:
            static constexpr int kBagSlots = 50; // player bag slots, 1-based
            static constexpr int kWithdrawSlots = 25; // guild bank slots shown for withdrawal, 1-based

            /// @brief Toggles the choice of a bag slot (only weapon materials can be chosen).
            /// @details Original: ToggleBankSlotSelect.
            /// @param slot Bag slot number (1..50).
            void press(int slot);

            /// @brief Sends the entered amounts per kind to the bank (39/52) and resets the boxes.
            /// @details Original: BankGoldLeftClick.
            void deposit();

            /// @brief Takes the chosen bag slots out of the bank (39/53).
            /// @details Original: BankGoldRightClick.
            void withdraw();

            /// @brief Paints the window decoration, the kind icons and the stored counts.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints a tint over the chosen bag slots.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paintChosen(gfx::Canvas& canvas, core::Point at);

            /// @brief Tells whether a bag slot holds a weapon material.
            /// @param slot Bag slot number.
            /// @return True when the item in the slot has item type 53.
            bool isMaterial(int slot) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_bag;
            const std::array<std::int32_t, kKinds + 1>& m_weaponCounts;
            ui::Window& m_window;
            ui::IconGrid& m_items;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_chosenSpot;
            std::vector<std::reference_wrapper<ui::TextBox>> m_amounts;
            std::array<bool, kBagSlots + 1> m_chosen{};
    };

}

#endif
