#include "formats/TalkTable.h"

#include "core/ByteReader.h"

#include <algorithm>
#include <cctype>

namespace formats {

    namespace {

        constexpr std::size_t kRecordSize = 292; // bytes per Talk.Dat record
        constexpr std::size_t kTextRoom = 254; // bytes reserved for the talk text (after its length byte)
        constexpr int kVersion = 3; // first word of the header record
        constexpr std::uint16_t kIdKey = 60650; // XOR key of the stored talk id (0xECEA)
        constexpr int kBias = 5; // subtracted from the id after the XOR

        /// @brief Decodes a stored talk id: (stored XOR 60650) - 5.
        /// @param stored The raw u16 from the record.
        /// @return The talk id.
        int decodeId(std::uint16_t stored) { return static_cast<std::uint16_t>((stored ^ kIdKey) - kBias); }

        /// @brief Replaces a "#P n /#P" tag in a line by the number n.
        /// @details Original: ExpandTalkNumberTag.
        /// @param text The talk line.
        /// @return The line with the tag expanded.
        std::string expandNumber(std::string text) {
            const std::size_t open = text.find("#P");
            const std::size_t close = text.find("/#P");
            if (open == std::string::npos || close == std::string::npos || close < open) {
                return text;
            }
            std::string number = text.substr(open + 2, close - open - 2); // 2 = length of "#P"
            const auto first = number.find_first_not_of(' ');
            const auto last = number.find_last_not_of(' ');
            number = first == std::string::npos ? std::string() : number.substr(first, last - first + 1);
            if (!number.empty()) {
                try {
                    number = std::to_string(std::stoi(number));
                } catch (const std::exception&) {
                    number = "0";
                }
            }
            text.erase(open, close + 3 - open); // 3 = length of "/#P"
            text.insert(open, number);
            return text;
        }

    }

    std::optional<TalkTable> TalkTable::parse(const core::Bytes& data) {
        if (data.size() < kRecordSize) {
            return std::nullopt;
        }
        core::ByteReader header(data);
        if (header.u16() != kVersion) {
            return std::nullopt;
        }
        TalkTable table;
        for (std::size_t offset = kRecordSize; offset + kRecordSize <= data.size(); offset += kRecordSize) {
            core::ByteReader reader(data, offset);
            const int id = decodeId(reader.u16());
            const std::size_t length = std::min<std::size_t>(reader.u8(), kTextRoom);
            std::string room = reader.text(kTextRoom);
            std::reverse(room.begin(), room.end());
            if (id != 0) {
                table.m_talks.emplace(id, expandNumber(room.substr(0, length)));
            }
        }
        return table;
    }

    const std::string& TalkTable::text(int talkId) const {
        static const std::string kNone;
        const auto found = m_talks.find(talkId);
        return found != m_talks.end() ? found->second : kNone;
    }

    bool TalkTable::setText(int talkId, const std::string& text) {
        const auto found = m_talks.find(talkId);
        if (found == m_talks.end()) {
            return false;
        }
        found->second = expandNumber(text);
        return true;
    }

}
