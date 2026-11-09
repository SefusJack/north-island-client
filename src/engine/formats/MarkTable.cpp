#include "formats/MarkTable.h"

#include "core/ByteReader.h"

#include <algorithm>

namespace formats {

    namespace {

        constexpr std::uint8_t kByteKey = 144; // XOR key for byte fields of Mark.Dat (0x90)
        constexpr std::uint16_t kWordKey = 10100; // XOR key for 16-bit fields of Mark.Dat (0x2774)
        constexpr int kBias = 7; // subtracted from every value after the XOR
        constexpr int kHeaderFlag = 4; // the header record holds the raw value 4 in its flag-bit field
        constexpr std::size_t kTextCapacity = 254; // bytes of the text field (after its length byte)

        /// @brief Reads the obfuscated fields of one Mark.Dat record.
        class RecordReader {
            public:
                /// @brief Creates a reader for the record starting at a byte offset.
                /// @param data Whole file bytes.
                /// @param start Offset of the record start.
                RecordReader(
                    const core::Bytes& data,
                    std::size_t start
                )
                    : m_reader(data),
                      m_start(start) {}

                /// @brief Reads a decoded byte field ((value ^ key) - 7).
                /// @param offset Offset inside the record.
                /// @return The decoded value.
                int byteAt(std::size_t offset) {
                    m_reader.seek(m_start + offset);
                    return static_cast<std::uint8_t>((m_reader.u8() ^ kByteKey) - kBias);
                }

                /// @brief Reads a decoded 16-bit field ((value ^ key) - 7).
                /// @param offset Offset inside the record.
                /// @return The decoded value.
                int wordAt(std::size_t offset) {
                    m_reader.seek(m_start + offset);
                    return static_cast<std::uint16_t>((m_reader.u16() ^ kWordKey) - kBias);
                }

                /// @brief Reads an undecoded 16-bit field.
                /// @param offset Offset inside the record.
                /// @return The raw value.
                int rawWordAt(std::size_t offset) {
                    m_reader.seek(m_start + offset);
                    return m_reader.u16();
                }

                /// @brief Reads a length-prefixed string[254] stored back to front.
                /// @param offset Offset inside the record (length byte first).
                /// @return The text in reading order.
                std::string reversedText(std::size_t offset) {
                    m_reader.seek(m_start + offset);
                    const std::size_t length = std::min<std::size_t>(m_reader.u8(), kTextCapacity);
                    std::string raw = m_reader.text(kTextCapacity);
                    std::reverse(raw.begin(), raw.end());
                    return raw.substr(0, std::min(length, raw.size()));
                }

            private:
                core::ByteReader m_reader;
                std::size_t m_start;
        };

    }

    std::optional<MarkTable> MarkTable::parse(const core::Bytes& data) {
        if (data.size() < kRecordSize * 2 || RecordReader(data, 0).rawWordAt(258) != kHeaderFlag) { // record offset 258: flag bit (raw in the header)
            return std::nullopt;
        }
        MarkTable table;
        for (std::size_t start = kRecordSize; start + kRecordSize <= data.size(); start += kRecordSize) {
            RecordReader reader(data, start);
            MarkRecord mark;
            mark.name = reader.reversedText(0);
            mark.markId = reader.wordAt(256); // record offset of the mark id
            mark.flagBit = reader.wordAt(258); // record offset of the flag bit
            mark.text = reader.reversedText(261); // record offset of the text
            mark.hiddenInList = reader.byteAt(517) != 0; // record offset of the hidden-in-task-list flag
            table.m_records.push_back(mark);
            const int index = static_cast<int>(table.m_records.size());
            if (mark.markId != 0) {
                table.m_indexById.emplace(mark.markId, index);
            }
            table.m_indexByFlag.emplace(mark.flagBit, index);
        }
        return table;
    }

    int MarkTable::indexOf(int markId) const {
        const auto found = m_indexById.find(markId);
        return found != m_indexById.end() ? found->second : 0;
    }

    int MarkTable::indexOfFlag(int flagBit) const {
        const auto found = m_indexByFlag.find(flagBit);
        return found != m_indexByFlag.end() ? found->second : 0;
    }

    std::optional<std::reference_wrapper<const MarkRecord>> MarkTable::at(int index) const {
        if (index < 1 || index > count()) {
            return std::nullopt;
        }
        return std::cref(m_records[static_cast<std::size_t>(index - 1)]);
    }

}
