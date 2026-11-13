#include "formats/MrgArchive.h"

#include "core/ByteReader.h"
#include "core/StringUtil.h"

#include <algorithm>
#include <utility>

namespace formats {

    namespace {

        constexpr std::size_t kEntrySize = 32; // bytes per table entry (name with length byte, offset and size)

        /// @brief File extension of an archive kind.
        /// @param kind Archive kind.
        /// @return The extension with its dot.
        const char* extensionFor(MrgArchive::Kind kind) {
            switch (kind) {
                case MrgArchive::Kind::Jpeg: // JMg archive: JPEG files
                    return ".jpg";
                case MrgArchive::Kind::Targa: // TMg archive: targa files
                    return ".tga";
                case MrgArchive::Kind::Bitmap: // BMg archive: bitmaps (falls through to .bmp)
                    break;
            }
            return ".bmp";
        }

    }

    std::optional<MrgArchive> MrgArchive::open(const std::filesystem::path& path) {
        MrgArchive archive;
        const std::string extension = core::toLower(path.extension().string());
        if (extension == ".bmg") {
            archive.m_kind = Kind::Bitmap;
        } else if (extension == ".jmg") {
            archive.m_kind = Kind::Jpeg;
        } else if (extension == ".tmg") {
            archive.m_kind = Kind::Targa;
        } else {
            return std::nullopt;
        }

        archive.m_file.open(path, std::ios::binary);
        if (!archive.m_file) {
            return std::nullopt;
        }
        core::Bytes countBytes(2); // the u16 entry count
        archive.m_file.read(reinterpret_cast<char*>(countBytes.data()), 2); // the u16 entry count
        const std::size_t count = static_cast<std::size_t>(countBytes[0] | (countBytes[1] << 8)); // little-endian: high byte shifted 8 bits
        core::Bytes table(count * kEntrySize);
        archive.m_file.read(reinterpret_cast<char*>(table.data()), static_cast<std::streamsize>(table.size()));
        if (!archive.m_file) {
            return std::nullopt;
        }
        core::ByteReader reader(table);
        for (std::size_t i = 0; i < count; ++i) {
            reader.seek(i * kEntrySize);
            const std::uint8_t length = reader.u8();
            Entry entry;
            entry.name = reader.text(23).substr(0, std::min<std::size_t>(length, 23)); // Pascal short string of up to 23 characters
            entry.offset = reader.u32();
            entry.size = reader.u32();
            archive.m_entries[core::toLower(entry.name)] = entry;
        }
        return std::optional<MrgArchive>(std::move(archive));
    }

    std::string MrgArchive::keyFor(const std::string& name) const { return core::toLower(name + extensionFor(m_kind)); }

    bool MrgArchive::contains(const std::string& name) const { return m_entries.count(keyFor(name)) != 0; }

    std::optional<core::Bytes> MrgArchive::read(const std::string& name) {
        const auto found = m_entries.find(keyFor(name));
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

    std::optional<core::Bytes> MrgArchive::readStart(const std::string& name, std::size_t most, std::size_t& fullSize) {
        const auto found = m_entries.find(keyFor(name));
        if (found == m_entries.end()) {
            return std::nullopt;
        }
        fullSize = found->second.size;
        core::Bytes data(std::min<std::size_t>(found->second.size, most));
        m_file.clear();
        m_file.seekg(found->second.offset);
        m_file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!m_file) {
            return std::nullopt;
        }
        return data;
    }

    std::vector<std::string> MrgArchive::names() const {
        std::vector<std::string> result;
        for (const auto& item : m_entries) {
            const std::string& name = item.second.name;
            const std::size_t dot = name.rfind('.');
            result.push_back(dot == std::string::npos ? name : name.substr(0, dot));
        }
        return result;
    }

}
