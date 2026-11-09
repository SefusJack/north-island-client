#include "formats/DataArchive.h"

#include "core/ByteReader.h"
#include "core/StringUtil.h"

#include <algorithm>
#include <utility>

namespace formats {

    namespace {

        constexpr std::size_t kEntrySize = 29; // bytes per table entry: 1 length byte + 20 name bytes + u32 offset + u32 size
        constexpr std::size_t kNameLength = 20; // bytes reserved for a file name in an entry

    }

    std::optional<DataArchive> DataArchive::open(const std::filesystem::path& path) {
        DataArchive archive;
        archive.m_file.open(path, std::ios::binary | std::ios::ate);
        if (!archive.m_file) {
            return std::nullopt;
        }
        const auto fileSize = static_cast<std::size_t>(archive.m_file.tellg());
        if (fileSize < 2) { // the entry count is the last 2 bytes
            return std::nullopt;
        }
        // u16 entry count: the last 2 bytes of the file, little-endian
        core::Bytes countBytes(2);
        archive.m_file.seekg(static_cast<std::streamoff>(fileSize - 2));
        archive.m_file.read(reinterpret_cast<char*>(countBytes.data()), 2);
        const std::size_t count = static_cast<std::size_t>(countBytes[0] | (countBytes[1] << 8));
        if (count * kEntrySize + 2 > fileSize) { // the entry table plus the 2-byte count must fit
            return std::nullopt;
        }
        core::Bytes table(count * kEntrySize);
        archive.m_file.seekg(static_cast<std::streamoff>(fileSize - 2 - table.size())); // the table ends just before the 2-byte count
        archive.m_file.read(reinterpret_cast<char*>(table.data()), static_cast<std::streamsize>(table.size()));
        if (!archive.m_file) {
            return std::nullopt;
        }
        core::ByteReader reader(table);
        for (std::size_t i = 0; i < count; ++i) {
            reader.seek(i * kEntrySize);
            const std::uint8_t length = reader.u8();
            const std::string name = reader.text(kNameLength).substr(0, std::min<std::size_t>(length, kNameLength));
            Entry entry;
            entry.offset = reader.u32();
            entry.size = reader.u32();
            archive.m_entries[core::toLower(name)] = entry;
        }
        return std::optional<DataArchive>(std::move(archive));
    }

    bool DataArchive::contains(const std::string& fileName) const { return m_entries.count(core::toLower(fileName)) != 0; }

    std::optional<core::Bytes> DataArchive::read(const std::string& fileName) {
        const auto found = m_entries.find(core::toLower(fileName));
        if (found == m_entries.end()) {
            return std::nullopt;
        }
        core::Bytes data(found->second.size);
        m_file.clear();
        m_file.seekg(found->second.offset);
        m_file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!m_file) {
            return std::nullopt;
        }
        return data;
    }

}
