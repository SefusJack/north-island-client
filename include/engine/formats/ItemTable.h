#ifndef NORTH_ISLAND_FORMATS_ITEM_TABLE_H
#define NORTH_ISLAND_FORMATS_ITEM_TABLE_H

#include "core/Bytes.h"
#include "formats/ItemRecord.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace formats {

    /// @brief The item table loaded from data\Item.dat.
    /// @details Original: 457-byte records, the first one a header. Names are stored reversed; every number is stored as
    /// (value + 9) XOR a key (byte 0x9A, word 0xEFC3, integer 0x0B80F4B4).
    class ItemTable {
        public:
            static constexpr std::size_t kRecordSize = 457; // bytes per item record in Item.dat

            /// @brief Parses Item.dat.
            /// @param data Raw file bytes.
            /// @return The table, or nothing when the file is malformed.
            static std::optional<ItemTable> parse(const core::Bytes& data);

            /// @brief Looks an item up by id.
            /// @param itemId Item id.
            /// @return The record, or nothing for unknown ids.
            std::optional<ItemRecord> find(int itemId) const;

            /// @brief Finds the first item whose linked skill is the given one.
            /// @details Original: GetItemIdBySkill.
            /// @param skillId Skill id.
            /// @return The record, or nothing when no item links that skill.
            std::optional<ItemRecord> findByLinkedSkill(int skillId) const;

            /// @brief Number of items.
            /// @return The item count.
            std::size_t size() const { return m_items.size(); }

            /// @brief Every item id, in order (the qol client's admin item browser).
            /// @return The ids.
            std::vector<int> ids() const;

            /// @brief Replaces an item's name and description from a text pack (the qol client's mods).
            /// @param itemId Item id.
            /// @param name New name, or nothing to keep it.
            /// @param description New description, or nothing to keep it.
            /// @return True when the item exists.
            bool setText(int itemId, const std::optional<std::string>& name, const std::optional<std::string>& description);

        private:
            std::unordered_map<int, ItemRecord> m_items;
            std::unordered_map<int, int> m_itemsBySkill;
    };

}

#endif
