#ifndef NORTH_ISLAND_FORMATS_EVENT_ARCHIVE_H
#define NORTH_ISLAND_FORMATS_EVENT_ARCHIVE_H

#include "core/Bytes.h"
#include "formats/EventScene.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>

namespace formats {

    /// @brief The maps' event scenes read from Data/eve.Emg.
    /// @details Original: TEventManage.LoadEveEmgFile, ReadEventScene. File: f64, i32 count, count x (u16 map, u16 source map, i32 offset, u16 size), the scenes. A scene: u16 map, u16 version (112), then sections of u16 count + count x (u16 index, record): NPCs, doors, mines, goods, events (u8, name[20], u8 n conditions of 22 bytes each with u8 m results of 22 bytes), groups, warps, interacts (name[20], u8 kind, u16 value, u8 n + n x 5 bytes), battles, ... Placed records start with string[19] name, u8 flags, i32 x, i32 y, u8 n + n event ids, u8 n + n bytes, u8.
    class EventArchive {
        public:
            /// @brief Reads the archive file and its index.
            /// @param path Path of eve.Emg.
            /// @return The archive, nothing when unreadable or malformed.
            static std::optional<EventArchive> open(const std::filesystem::path& path);

            /// @brief Decodes the scene of a map; maps without one use map 12003's.
            /// @param mapId Map number.
            /// @return The scene, nothing when absent or malformed.
            /// @details Original: LoadEventFile.
            std::optional<EventScene> scene(int mapId) const;

        private:
            /// @brief Where a map's scene lies in the file.
            struct Entry {
                std::size_t offset = 0;
                std::size_t size = 0;
            };

            core::Bytes m_data;
            std::map<int, Entry> m_entries;
    };

}

#endif
