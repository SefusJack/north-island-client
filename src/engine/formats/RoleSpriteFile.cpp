#include "formats/RoleSpriteFile.h"

#include "core/ByteReader.h"
#include "core/FileSystem.h"
#include "core/StringUtil.h"

#include <utility>

namespace formats {

    namespace {

        constexpr std::size_t kIndexRecordSize = 28; // bytes per .jma index record: 20-byte name + u32 size + u32 offset
        constexpr std::size_t kFrameRecordSize = 40; // bytes per frame record inside a sprite block
        constexpr std::size_t kJxaRecordSize = 80; // bytes per .jxa record: per-slot frame counts

        /// @brief Reads a byte range of the open file.
        /// @param file Open input stream.
        /// @param offset Start offset.
        /// @param size Number of bytes.
        /// @return The bytes, or an empty buffer when the read fails.
        core::Bytes readAt(std::ifstream& file, std::size_t offset, std::size_t size) {
            core::Bytes data(size);
            file.clear();
            file.seekg(static_cast<std::streamoff>(offset));
            file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
            if (!file) {
                return {};
            }
            return data;
        }

    }

    std::optional<RoleSpriteFile> RoleSpriteFile::open(const std::filesystem::path& jmaFile, const std::filesystem::path& jxaFile) {
        RoleSpriteFile bank;
        bank.m_file.open(jmaFile, std::ios::binary);
        const auto jxa = core::readFile(jxaFile);
        if (!bank.m_file || !jxa) {
            return std::nullopt;
        }
        const core::Bytes header = readAt(bank.m_file, 0, 14); // .jma header: "ja", file size, first block offset, count = 14 bytes
        if (header.size() != 14 || header[0] != 'j' || header[1] != 'a') { // 14: header size; magic "ja"
            return std::nullopt;
        }
        core::ByteReader headerReader(header, 10); // offset of the u32 entry count in the header
        const std::size_t count = headerReader.u32();
        const core::Bytes index = readAt(bank.m_file, 14, count * kIndexRecordSize); // the index follows the 14-byte header
        if (index.size() != count * kIndexRecordSize) {
            return std::nullopt;
        }
        core::ByteReader indexReader(index);
        core::ByteReader jxaReader(*jxa);
        for (std::size_t i = 0; i < count; ++i) {
            indexReader.seek(i * kIndexRecordSize);
            // index record: a 20-byte short string (length byte + up to 19 characters), then u32 block size and u32 block offset
            const std::string name = indexReader.text(20);
            const std::size_t length = std::min<std::size_t>(static_cast<unsigned char>(name[0]), 19);
            std::string stem = name.substr(1, length);
            stem = stem.substr(0, stem.find('.'));
            Entry entry;
            entry.size = indexReader.u32();
            entry.offset = indexReader.u32();
            jxaReader.seek(14 + i * kJxaRecordSize); // .Jxa records start after its 14-byte header
            const core::Bytes counts = jxaReader.bytes(kJxaRecordSize);
            if (counts.size() == kJxaRecordSize) {
                // 79: index of the last-slot byte of an 80-byte record and the most slots (0..78)
                const std::size_t slots = std::min<std::size_t>(std::size_t{counts[79]} + 1, 79);
                for (std::size_t s = 0; s < slots; ++s) {
                    entry.slotFrameCounts.push_back(counts[s]);
                }
            }
            if (const auto id = core::parseInt(stem)) {
                bank.m_entries[*id] = std::move(entry);
            }
        }
        return std::optional<RoleSpriteFile>(std::move(bank));
    }

    std::optional<RoleSprite> RoleSpriteFile::read(int spriteId) {
        const auto found = m_entries.find(spriteId);
        if (found == m_entries.end()) {
            return std::nullopt;
        }
        const core::Bytes block = readAt(m_file, found->second.offset, found->second.size);
        if (block.size() < 14 || block[0] != 'j' || block[1] != 'm') { // 14: block header size; magic "jm"
            return std::nullopt;
        }
        core::ByteReader reader(block, 10); // offset of the u32 frame count in the block header
        const std::size_t frameCount = reader.u32();
        const std::size_t paletteStart = 14 + frameCount * kFrameRecordSize; // the palette follows the 14-byte header and the frame records
        if (paletteStart + 1024 > block.size()) { // 1024: 256 palette entries of 4 bytes
            return std::nullopt;
        }

        RoleSprite sprite;
        sprite.slotFrameCounts = found->second.slotFrameCounts;
        reader.seek(paletteStart);
        for (core::Color& color : sprite.palette) {
            color.b = reader.u8();
            color.g = reader.u8();
            color.r = reader.u8();
            reader.u8();
            color.a = 255; // opaque
        }
        for (std::size_t k = 0; k < frameCount; ++k) {
            reader.seek(14 + k * kFrameRecordSize + 12); // frame records start after the 14-byte header; +12 skips the short string[11] name
            RoleSpriteFrame frame;
            frame.canvasWidth = reader.u16();
            frame.canvasHeight = reader.u16();
            frame.offsetX = reader.u16();
            frame.offsetY = reader.u16();
            reader.u32();
            const std::size_t pixelOffset = reader.u32();
            frame.width = static_cast<int>(reader.u32());
            frame.height = static_cast<int>(reader.u32());
            const std::size_t stride = (static_cast<std::size_t>(frame.width) + 3) & ~static_cast<std::size_t>(3); // rows are padded to 4 bytes
            if (reader.failed() || frame.width <= 0 || frame.height <= 0 || pixelOffset + stride * static_cast<std::size_t>(frame.height) > block.size()) {
                sprite.frames.push_back(RoleSpriteFrame{});
                continue;
            }
            frame.pixels.reserve(static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height));
            for (int y = 0; y < frame.height; ++y) {
                const auto row = block.begin() + static_cast<std::ptrdiff_t>(pixelOffset + stride * static_cast<std::size_t>(y));
                frame.pixels.insert(frame.pixels.end(), row, row + frame.width);
            }
            sprite.frames.push_back(std::move(frame));
        }
        return sprite;
    }

}
