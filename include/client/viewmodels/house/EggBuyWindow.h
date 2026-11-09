#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_EGG_BUY_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_EGG_BUY_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief A Fortune Egg machine seen by a visitor (EggBuyWindow.xml, TTH_BuyEggMachineForm): the ten places, the price of a play and the eggs it holds.
    /// @details Original: TH_Menu.pas TTH_BuyEggMachineForm. OK asks to play it at that price, after a question (62/33).
    class EggBuyWindow {
        public:
            /// @brief Callbacks into the rest of the client that the egg buy window needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<void(const std::string&, std::function<void()>)> askTimed;
                std::function<std::int32_t()> gold;
            };

            /// @brief Loads the window layout and wires the paint area and buttons.
            /// @param services Game services (layouts, item records).
            /// @param screen Screen the window is loaded into.
            /// @param house The house interior with its utensils and stalls.
            /// @param inventory The player's bag.
            /// @param messages Player message line for refusals.
            /// @param hooks Callbacks into the rest of the client.
            EggBuyWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const Inventory& inventory,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the machine of a piece: the price in the colour of its digits, and asks the machine for what it holds (62/36).
            /// @details Original: ShowCtrl.
            /// @param furnIndex Furniture piece index of the machine.
            void open(int furnIndex);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Handles a price change of the machine (62/32); a machine no longer for sale is empty and the window goes.
            /// @param furnIndex Furniture piece index of the machine.
            /// @param price New price of a play.
            void priceChanged(int furnIndex, std::int32_t price);

        private:
            /// @brief Checks room in the bag, a set price and the gold, then asks the question and sends the play request (62/33).
            /// @details Original: PlayEggMachineClick.
            void play();

            /// @brief Shows the price in the colour its digits give.
            /// @param price Price to show.
            void showPrice(std::int32_t price);

            /// @brief Gives the house utensil of the shown machine.
            /// @return The machine's utensil record.
            const HouseUtensil& utensil() const;

            /// @brief Paints the egg count and fills the icon cells with the stacks in the machine's places; closes the form out of the house.
            /// @details Original: DrawControl: the eggs the machine holds (0 when it is not selling) and the stacks in its places.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_price;
            ui::IconGrid& m_cells;
            const ui::Spot& m_eggCountSpot;
            int m_furnIndex = 0;
    };

}

#endif
