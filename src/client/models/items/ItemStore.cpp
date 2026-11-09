#include "client/models/items/ItemStore.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kMaxStack = 50; // most items of one kind in a single stackable bag cell

    }

    void ItemStore::add(const protocol::BagItemRecord& record, bool stackable) {
        protocol::ItemInstance rest = record.item;
        for (int tries = 0; rest.count > 0 && tries <= slots(); ++tries) {
            int slot = record.slot;
            if (slot == 0) {
                const auto isFree = [](const protocol::ItemInstance& there) { return there.empty(); };
                const auto stack = std::find_if(m_items.begin() + 1, m_items.end(), [&rest](const protocol::ItemInstance& there) {
                    return there.itemId == rest.itemId && there.count > 0 && there.count < kMaxStack;
                });
                const auto found = stackable && stack != m_items.end() ? stack : std::find_if(m_items.begin() + 1, m_items.end(), isFree);
                if (found == m_items.end()) {
                    break;
                }
                slot = static_cast<int>(found - m_items.begin());
            }
            if (slot < 1 || slot > slots()) {
                break;
            }
            protocol::ItemInstance& there = m_items[static_cast<std::size_t>(slot)];
            int added = 0;
            if (there.empty()) {
                there = rest;
                there.count = std::min(rest.count, kMaxStack);
                added = there.count;
            } else if (there.itemId == rest.itemId) {
                added = std::min(rest.count, kMaxStack - there.count);
                there.count += added;
            }
            rest.count -= added;
            if (record.slot != 0 || added == 0) {
                break;
            }
        }
    }

    void ItemStore::clear(int slot) {
        if (slot >= 1 && slot <= slots()) {
            m_items[static_cast<std::size_t>(slot)] = {};
        }
    }

}
