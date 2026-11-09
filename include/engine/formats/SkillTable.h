#ifndef NORTH_ISLAND_FORMATS_SKILL_TABLE_H
#define NORTH_ISLAND_FORMATS_SKILL_TABLE_H

#include "core/Bytes.h"
#include "formats/SkillRecord.h"

#include <optional>
#include <string>
#include <unordered_map>

namespace formats {

    /// @brief The skills of data\Skill.dat.
    /// @details Original: 148-byte records, the first one a header (its byte 21 is 11). Names and descriptions are
    /// stored reversed; every number is stored as (value + 4) XOR a key (byte 0xFD, word 0x6EA0, integer 0x0BDEE4BF).
    class SkillTable {
        public:
            static constexpr std::size_t kRecordSize = 148; // bytes per skill record in Skill.dat

            /// @brief Decodes Skill.dat.
            /// @param data The raw file bytes.
            /// @return The table, or nothing when the file is too short or its header record is wrong.
            static std::optional<SkillTable> parse(const core::Bytes& data);

            /// @brief Looks a skill up by its id.
            /// @param skillId The skill id.
            /// @return The record, or nothing for an unknown id.
            std::optional<SkillRecord> find(int skillId) const;

            /// @brief Looks a skill up by the key the server keeps its level under.
            /// @param key The skill key (1..500).
            /// @return The record, or nothing for an unknown key.
            std::optional<SkillRecord> findByKey(int key) const;

            /// @brief The number of skills in the table.
            /// @return The skill count.
            std::size_t size() const { return m_skills.size(); }

            /// @brief Replaces a skill's name and description from a text pack (the qol client's mods).
            /// @param skillId The skill id.
            /// @param name The new name, if any.
            /// @param description The new description, if any.
            /// @return Whether the skill is in the table.
            bool setText(int skillId, const std::optional<std::string>& name, const std::optional<std::string>& description);

        private:
            std::unordered_map<int, SkillRecord> m_skills;
            std::unordered_map<int, int> m_idsByKey;
    };

}

#endif
