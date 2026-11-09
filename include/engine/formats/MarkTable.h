#ifndef NORTH_ISLAND_FORMATS_MARK_TABLE_H
#define NORTH_ISLAND_FORMATS_MARK_TABLE_H

#include "core/Bytes.h"
#include "formats/MarkRecord.h"

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

namespace formats {

    /// @brief The quest mark table loaded from data\Mark.Dat.
    /// @details Original: TFMark; 553-byte records, the first a header whose flag bit is 4 before decoding. Names and texts are
    /// 254-character strings stored reversed; numbers are stored as (value + 7) XOR a key (byte 144, word 10100). Records keep their
    /// file order (1..count): a counter mark's quest is the first flagged mark from it on.
    class MarkTable {
        public:
            static constexpr std::size_t kRecordSize = 553; // bytes per mark record in Mark.Dat

            /// @brief Parses Mark.Dat.
            /// @param data Raw file bytes.
            /// @return The table, or nothing when the file is too small or the header flag is wrong.
            static std::optional<MarkTable> parse(const core::Bytes& data);

            /// @brief Place of a mark by id.
            /// @details Original: GetMarkIndex.
            /// @param markId Mark id.
            /// @return The place 1..count, or 0 for none.
            int indexOf(int markId) const;

            /// @brief Place of a mark by flag bit.
            /// @details Original: FindMarkByFlagBit.
            /// @param flagBit Flag bit.
            /// @return The place 1..count, or 0 for none.
            int indexOfFlag(int flagBit) const;

            /// @brief The record at a place.
            /// @param index Place, 1..count.
            /// @return The record, or nothing when out of range.
            std::optional<std::reference_wrapper<const MarkRecord>> at(int index) const;

            /// @brief The record of a mark id.
            /// @param markId Mark id.
            /// @return The record, or nothing for unknown ids.
            std::optional<std::reference_wrapper<const MarkRecord>> find(int markId) const { return at(indexOf(markId)); }

            /// @brief Number of records (without the header).
            /// @return The count.
            int count() const { return static_cast<int>(m_records.size()); }

        private:
            std::vector<MarkRecord> m_records;
            std::unordered_map<int, int> m_indexById;
            std::unordered_map<int, int> m_indexByFlag;
    };

}

#endif
