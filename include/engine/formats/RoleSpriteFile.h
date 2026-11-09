#ifndef NORTH_ISLAND_FORMATS_ROLE_SPRITE_FILE_H
#define NORTH_ISLAND_FORMATS_ROLE_SPRITE_FILE_H

#include "formats/RoleSprite.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <vector>

namespace formats {

    /// @brief A sprite bank: jma\NNN.jma (the sprites) with jma\NNN.Jxa (frames per action slot).
    /// @details .jma: "ja", u32 file size, u32 first block offset, u32 count; count x 28-byte index records (short string name "2000.jmp",
    /// u32 block size, u32 block offset). A block: "jm", u32 pixel bytes, u32 pixel start, u32 frame count nf; nf x 40-byte frame records
    /// (short string[11], u16 canvas width, u16 canvas height, u16 offset x, u16 offset y, u32 image size, u32 pixel offset in the block,
    /// u32 width, u32 height, u32); a 256-entry palette (B, G, R, 0); the pixels (one byte each, rows top-down padded to 4 bytes).
    /// .Jxa: "jx", u32 size, u32 14, u32 count; count x 80 bytes (frames of slots 0..78, then the number of slots), in the order of the .jma index.
    /// Sprites are named by number ("2000.jmp" is sprite 2000).
    class RoleSpriteFile {
        public:
            /// @brief Opens a bank and reads its sprite index.
            /// @param jmaFile Path of the .jma file.
            /// @param jxaFile Path of the .Jxa file.
            /// @return The bank, or nothing when a file is missing or malformed.
            static std::optional<RoleSpriteFile> open(const std::filesystem::path& jmaFile, const std::filesystem::path& jxaFile);

            /// @brief Whether the bank has a sprite.
            /// @param spriteId Sprite number.
            /// @return True when present.
            bool contains(int spriteId) const { return m_entries.count(spriteId) != 0; }

            /// @brief Lists the sprites of the bank.
            /// @return The sprite numbers, in order.
            std::vector<int> ids() const {
                std::vector<int> ids;
                for (const auto& entry : m_entries) {
                    ids.push_back(entry.first);
                }
                return ids;
            }

            /// @brief Reads a sprite's palette and frames.
            /// @param spriteId Sprite number.
            /// @return The sprite, or nothing when missing or malformed.
            std::optional<RoleSprite> read(int spriteId);

        private:
            /// @brief One sprite's block position and per-slot frame counts.
            struct Entry {
                std::uint32_t offset = 0;
                std::uint32_t size = 0;
                std::vector<int> slotFrameCounts;
            };

            std::ifstream m_file;
            std::map<int, Entry> m_entries;
    };

}

#endif
