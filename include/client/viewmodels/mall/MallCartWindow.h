#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MALL_MALL_CART_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MALL_MALL_CART_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/Inventory.h"
#include "client/models/mall/MallShop.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief Window of the mall's shopping cart: rows, counts, delete, total and payment.
    /// @details Original: MallCartWindow.xml, THL_ShoppingCarForm. Pay asks for the IM points (34/1 1) and their
    /// answer buys the cart (BuyCart, 75/1) when the points and the bag allow.
    class MallCartWindow {
        public:
            /// @brief Callbacks into the battle state and the mall window.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<void()> showMall;
                std::function<void()> hideMall;
            };

            /// @brief Loads the window layout and wires its buttons.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param shop Mall shop state (the cart lives here).
            /// @param status Player status (level).
            /// @param inventory Player bag, checked for room.
            /// @param messages Player message line.
            /// @param hooks Callbacks into the surrounding forms.
            MallCartWindow(
                GameServices& services,
                ui::Screen& screen,
                MallShop& shop,
                const PlayerStatus& status,
                const Inventory& inventory,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the cart and hides the mall (not in a battle).
            /// @details Original: ShowCtrl (the mall hides).
            void show();

            /// @brief Hides the window.
            void hide() { m_window.setVisible(false); }

            /// @brief Opens the cart when closed, closes it when open.
            /// @details Original: ToggleShoppingCarForm.
            void toggle();

            /// @brief Handles the IM points answer: with a payment waiting the cart is bought.
            /// @details Original: BuyCart. Answer 35/4 to the points request 34/1; the purchase is sent as 75/1.
            void pointsArrived();

            /// @brief Refreshes counts after a purchase, reads typed counts and hides the cart in a battle.
            /// @details Original: DrawControl's refresh.
            void update();

        private:
            /// @brief Removes a cart line.
            /// @details Original: RemoveCartEntry.
            /// @param line 1-based cart line.
            void remove(int line);

            /// @brief Asks for the IM points so the cart can be paid.
            /// @details Original: ConfirmCartPayment (34/1 1).
            void pay();

            /// @brief Resets the count boxes from the cart: an emptied place clears, a new line counts one.
            /// @details Original: RefreshCartCounts.
            void refreshCounts();

            /// @brief Reads the typed counts (fixed or typed, 0..50) into the cart lines.
            /// @details Original: the counts DrawControl keeps.
            void readCounts();

            /// @brief Checks that the bag has room for the whole cart (furniture is not built).
            /// @details Original: CheckCartSpace.
            /// @return True when everything fits.
            bool bagHasRoom() const;

            /// @brief Paints the cart rows, pictures, prices and total.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            MallShop& m_shop;
            const PlayerStatus& m_status;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::ImageButton>, MallShop::kCartLines> m_delete;
            std::array<std::reference_wrapper<ui::TextBox>, MallShop::kCartLines> m_counts;
    };

}

#endif
