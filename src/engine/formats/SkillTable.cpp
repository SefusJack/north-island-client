#include "formats/SkillTable.h"

#include "core/ByteReader.h"

#include <algorithm>

namespace formats {

    namespace {

        constexpr std::uint8_t kByteKey = 0xFD; // XOR key for byte fields of Skill.dat
        constexpr std::uint16_t kWordKey = 0x6EA0; // XOR key for 16-bit fields of Skill.dat
        constexpr std::uint8_t kHeaderMark = 11; // raw value at byte 21 of the header record
        constexpr int kBias = 4; // subtracted from every value after the XOR

        /// @brief Decodes the keyed fields of one 148-byte Skill.dat record.
        class RecordReader {
            public:
                /// @brief Starts reading a record.
                /// @param data The whole file.
                /// @param start Byte offset of the record in the file.
                RecordReader(
                    const core::Bytes& data,
                    std::size_t start
                )
                    : m_reader(data),
                      m_start(start) {}

                /// @brief Reads a byte field: (value XOR 0xFD) - 4.
                /// @param offset Offset inside the record.
                /// @return The decoded value.
                int byteAt(std::size_t offset) {
                    m_reader.seek(m_start + offset);
                    return static_cast<std::uint8_t>((m_reader.u8() ^ kByteKey) - kBias);
                }

                /// @brief Reads a 16-bit field: (value XOR 0x6EA0) - 4.
                /// @param offset Offset inside the record.
                /// @return The decoded value.
                int wordAt(std::size_t offset) {
                    m_reader.seek(m_start + offset);
                    return static_cast<std::uint16_t>((m_reader.u16() ^ kWordKey) - kBias);
                }

                /// @brief Reads a length-prefixed string stored reversed.
                /// @param offset Offset of the length byte inside the record.
                /// @param capacity Number of characters reserved for the text.
                /// @return The text in reading order, cut to its stored length.
                std::string reversedText(std::size_t offset, std::size_t capacity) {
                    m_reader.seek(m_start + offset);
                    const std::size_t length = std::min<std::size_t>(m_reader.u8(), capacity);
                    std::string raw = m_reader.text(capacity);
                    std::reverse(raw.begin(), raw.end());
                    return raw.substr(0, std::min(length, raw.size()));
                }

            private:
                core::ByteReader m_reader;
                std::size_t m_start;
        };

    }

    std::optional<SkillTable> SkillTable::parse(const core::Bytes& data) {
        if (data.size() < kRecordSize * 2 || data[21] != kHeaderMark) { // header record byte 21 holds the mark 11
            return std::nullopt;
        }
        SkillTable table;
        for (std::size_t start = kRecordSize; start + kRecordSize <= data.size(); start += kRecordSize) {
            RecordReader record(data, start);
            SkillRecord skill;
            // field offsets inside the 148-byte record (name 20 chars at 0, description 30 chars at 66; the rest are keyed bytes/words)
            skill.name = record.reversedText(0, 20); // name at offset 0, capacity 20
            skill.id = record.wordAt(22); // record offset of the skill id
            skill.category = record.byteAt(21); // record offset of the category
            skill.spCost = record.wordAt(24); // record offset of the SP cost
            skill.tree = record.byteAt(26); // record offset of the skill tree
            skill.rangeKind = record.wordAt(27); // record offset of the range kind
            skill.skillType = record.byteAt(29); // record offset of the skill type
            skill.learnPoints = record.byteAt(53); // record offset of the learn points
            skill.prerequisite = record.wordAt(54); // record offset of the prerequisite skill
            skill.description = record.reversedText(66, 30); // description at offset 66, capacity 30
            skill.key = record.wordAt(97); // record offset of the skill key
            skill.targetType = record.byteAt(99); // record offset of the target type
            skill.iconId = record.wordAt(100); // record offset of the icon picture id
            skill.maxLevel = record.byteAt(105); // record offset of the max level
            skill.statusEffect = record.byteAt(52); // record offset of the status effect
            skill.motionId = record.wordAt(102); // record offset of the motion id
            skill.statusKind = record.byteAt(104); // record offset of the status kind
            skill.tintKind = record.byteAt(106); // record offset of the tint kind
            if (skill.id == 0) {
                continue;
            }
            if (skill.key != 0) {
                table.m_idsByKey[skill.key] = skill.id;
            }
            table.m_skills[skill.id] = skill;
        }
        return table;
    }

    std::optional<SkillRecord> SkillTable::find(int skillId) const {
        const auto found = m_skills.find(skillId);
        if (found == m_skills.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    std::optional<SkillRecord> SkillTable::findByKey(int key) const {
        const auto found = m_idsByKey.find(key);
        if (found == m_idsByKey.end()) {
            return std::nullopt;
        }
        return find(found->second);
    }

    bool SkillTable::setText(int skillId, const std::optional<std::string>& name, const std::optional<std::string>& description) {
        const auto found = m_skills.find(skillId);
        if (found == m_skills.end()) {
            return false;
        }
        if (name) {
            found->second.name = *name;
        }
        if (description) {
            found->second.description = *description;
        }
        return true;
    }

}
