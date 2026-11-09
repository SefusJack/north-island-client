#ifndef NORTH_ISLAND_CLIENT_MODELS_MALL_MALL_SHOP_H
#define NORTH_ISLAND_CLIENT_MODELS_MALL_MALL_SHOP_H

#include "client/models/mall/MallCartLine.h"
#include "formats/ItemTable.h"
#include "protocol/MallRow.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief The item mall's catalog and cart: rows sorted into tabs, the page shown, the cart, the IM points and the pending payment.
    /// @details Original: THL_ShoppingManager. The 75/1 rows go into the tabs (1 the marked rows, 2 armour, 3 weapons, 4 goods, 5 furniture, each with its most); the cart holds up to seven lines.
    class MallShop {
        public:
            static constexpr int kTabs = 5;
            static constexpr int kCartLines = 7;
            static constexpr int kPageRows = 8;

            /// @brief Creates an empty mall.
            /// @param items Item table (unlisted items are left out).
            explicit MallShop(
                const formats::ItemTable& items
            )
                : m_items(items) {}

            /// @brief Sorts the server's rows into the tabs; the cart empties too.
            /// @details Original: RecvShopList / ParseShopList (75/1).
            /// @param rows The catalog rows.
            void load(const std::vector<protocol::MallRow>& rows);

            /// @brief Takes a new list, keeping the cart's lines the list still has with the same price and discount.
            /// @details Original: RecvShopListUpdate.
            /// @param rows The new catalog rows.
            /// @return True when some cart line was dropped.
            bool update(const std::vector<protocol::MallRow>& rows);

            /// @brief Whether a catalog was received.
            /// @return True when loaded.
            bool loaded() const { return m_loaded; }

            /// @brief The rows of a tab.
            /// @param tab Tab 1..5.
            /// @return The rows.
            const std::vector<protocol::MallRow>& tab(int tab) const { return m_tabs.at(static_cast<std::size_t>(tab)); }

            /// @brief Number of pages of a tab.
            /// @param tab Tab 1..5.
            /// @return The page count, at least 1.
            int pageCount(int tab) const;

            /// @brief Tells whether a row is in the catalog.
            /// @details Original: IsShopItemListed.
            /// @param itemId Item id.
            /// @param category Row category.
            /// @param count Item count of the row.
            /// @return True when listed.
            bool listed(int itemId, int category, int count) const;

            /// @brief Puts a row into the cart (the caller has checked the room left).
            /// @details Original: AddCartItem.
            /// @param row The catalog row.
            /// @return False when the row is not listed.
            bool addToCart(const protocol::MallRow& row);

            /// @brief Removes a cart line.
            /// @details Original: RemoveCartEntry.
            /// @param line Line place 1..7.
            void removeFromCart(int line);

            /// @brief The line bought leaves the cart.
            /// @details Original: RecvPurchaseResult.
            /// @param itemId Item id.
            /// @param category Row category.
            /// @param count Count of the line.
            /// @param key Row key.
            void bought(int itemId, int category, int count, int key);

            /// @brief The cart lines.
            /// @return The lines.
            std::vector<MallCartLine>& cart() { return m_cart; }

            /// @brief The cart lines (read only).
            /// @return The lines.
            const std::vector<MallCartLine>& cart() const { return m_cart; }

            /// @brief What the cart costs: each line's price less its discount (rounded half up) times its count.
            /// @return The total in IM points.
            int cartTotal() const;

            /// @brief Price of one item of a row after its discount, rounded half up.
            /// @param row The catalog row.
            /// @return The price.
            static int linePrice(const protocol::MallRow& row);

            int page = 1;
            std::int32_t points = 0;
            bool paymentPending = false;
            bool purchaseDone = false;
            bool gameAsked = false;

        private:
            const formats::ItemTable& m_items;
            std::array<std::vector<protocol::MallRow>, kTabs + 1> m_tabs;
            std::vector<MallCartLine> m_cart;
            bool m_loaded = false;
    };

}

#endif
