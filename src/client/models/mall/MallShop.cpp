#include "client/models/mall/MallShop.h"

#include <cmath>

namespace client {

    namespace {

        // most rows per tab, indexed by tab number (index 0 unused): tab 1 hot 64, tab 2 armour 48, tab 3 weapons 48, tab 4 goods 120, tab 5 furniture 24
        constexpr std::array<std::size_t, MallShop::kTabs + 1> kMostRows{0, 64, 48, 48, 120, 24};

    }

    void MallShop::load(const std::vector<protocol::MallRow>& rows) {
        m_cart.clear();
        for (auto& tab : m_tabs) {
            tab.clear();
        }
        page = 1;
        paymentPending = false;
        for (const protocol::MallRow& row : rows) {
            const auto item = m_items.find(row.itemId);
            if (!item || item->unlisted != 0 || row.key == 0) {
                continue;
            }
            int tab = 0;
            switch (row.category) {
                case 2: tab = 2; break; // category 2: armour, tab 2
                case 1: tab = 3; break; // category 1: weapons, tab 3
                case 3: // category 3: goods, tab 4
                case 4: tab = 4; break; // category 4: goods, tab 4
                case 5: tab = 5; break; // category 5: furniture, tab 5
                default: break; // other categories are not shown
            }
            if (tab != 0 && m_tabs[static_cast<std::size_t>(tab)].size() < kMostRows[static_cast<std::size_t>(tab)]) {
                m_tabs[static_cast<std::size_t>(tab)].push_back(row);
            }
        }
        // source tabs 4, 2, 3, 5 in this order; goods (tab 4) take marks up to 4, the others up to 3
        for (const int from : {4, 2, 3, 5}) {
            const int mostMark = from == 4 ? 4 : 3;
            for (const protocol::MallRow& row : m_tabs[static_cast<std::size_t>(from)]) {
                if (row.hot >= 1 && row.hot <= mostMark && m_tabs[1].size() < kMostRows[1]) {
                    m_tabs[1].push_back(row);
                }
            }
        }
        m_loaded = true;
    }

    bool MallShop::update(const std::vector<protocol::MallRow>& rows) {
        const std::vector<MallCartLine> before = std::move(m_cart);
        load(rows);
        for (const MallCartLine& line : before) {
            // category to tab: 2 armour -> 2, 1 weapons -> 3, 5 furniture -> 5, others -> 4 (goods)
            const int tab = line.row.category == 2 ? 2 : line.row.category == 1 ? 3 : line.row.category == 5 ? 5 : 4;
            for (const protocol::MallRow& row : m_tabs[static_cast<std::size_t>(tab)]) {
                if (row.itemId == line.row.itemId && row.category == line.row.category && row.price == line.row.price && row.discount == line.row.discount) {
                    m_cart.push_back({row, line.count});
                    break;
                }
            }
        }
        purchaseDone = true;
        return m_cart.size() != before.size();
    }

    int MallShop::pageCount(int tab) const {
        if (tab < 1 || tab > kTabs) {
            return 1;
        }
        return (static_cast<int>(m_tabs[static_cast<std::size_t>(tab)].size()) - 1) / kPageRows + 1;
    }

    bool MallShop::listed(int itemId, int category, int count) const {
        if (!m_loaded || itemId == 0) {
            return false;
        }
        const auto in = [&](int tab, bool sameCategory) {
            for (const protocol::MallRow& row : m_tabs[static_cast<std::size_t>(tab)]) {
                if (row.itemId == itemId && row.count == count && (!sameCategory || row.category == category)) {
                    return true;
                }
            }
            return false;
        };
        // category 2 armour is tab 2, 1 weapons tab 3, 5 furniture tab 5 (same category required); anything else is looked up in tab 4
        if ((category == 2 && in(2, false)) || (category == 1 && in(3, false)) || (category == 5 && in(5, true))) {
            return true;
        }
        return in(4, true);
    }

    bool MallShop::addToCart(const protocol::MallRow& row) {
        if (row.itemId == 0 || !listed(row.itemId, row.category, row.count)) {
            return false;
        }
        m_cart.push_back({row, 1});
        return true;
    }

    void MallShop::removeFromCart(int line) {
        if (line >= 1 && line <= static_cast<int>(m_cart.size())) {
            m_cart.erase(m_cart.begin() + (line - 1));
        }
    }

    void MallShop::bought(int itemId, int category, int count, int key) {
        for (auto line = m_cart.begin(); line != m_cart.end(); ++line) {
            if (line->row.itemId == itemId && line->row.category == category && line->row.key == key && line->count == count) {
                m_cart.erase(line);
                break;
            }
        }
        purchaseDone = true;
    }

    int MallShop::linePrice(const protocol::MallRow& row) {
        // the discount is a percentage; +0.5 rounds half up
        return static_cast<int>(std::floor(row.discount * row.price / 100.0 + 0.5));
    }

    int MallShop::cartTotal() const {
        int total = 0;
        for (const MallCartLine& line : m_cart) {
            total += linePrice(line.row) * line.count;
        }
        return total;
    }

}
