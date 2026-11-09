#ifndef NORTH_ISLAND_FORMATS_MOTION_TABLE_H
#define NORTH_ISLAND_FORMATS_MOTION_TABLE_H

#include "core/Bytes.h"
#include "formats/MotionRecord.h"

#include <optional>
#include <unordered_map>

namespace formats {

    /// @brief The battle motion table loaded from data\SkillData.MBTM.
    /// @details Original: CH_TMotionSkill; the motion records one after the other, then an index of 10-byte entries
    /// (u16 id, i32 offset, i32 size) and the u16 number of entries at the end. Numbers are little endian and not obscured; points are two i32 (x, y).
    class MotionTable {
        public:
            /// @brief Parses SkillData.MBTM.
            /// @param data Raw file bytes.
            /// @return The table, or nothing when the file is too small for its index.
            static std::optional<MotionTable> parse(const core::Bytes& data);

            /// @brief The motion of an id.
            /// @param id Motion id.
            /// @return The record, or nothing for unknown ids.
            std::optional<std::reference_wrapper<const MotionRecord>> find(int id) const;

            /// @brief Number of motions.
            /// @return The record count.
            std::size_t size() const { return m_records.size(); }

        private:
            std::unordered_map<int, MotionRecord> m_records;
    };

}

#endif
