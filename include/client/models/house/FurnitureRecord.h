#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_RECORD_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_RECORD_H

#include "core/ByteReader.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief One piece of a house's furniture as the server sends it: item, cell (x, y and level z, 1 the floor), facing, parent piece, linked pieces, room and retained flag.
    /// @details Original: TFurnitureRec, 57 bytes. Linked pieces are the ones placed on it (the tools on a Work Platform); retained means kept in the retained list instead of standing in the room.
    struct FurnitureRecord {
        static constexpr std::size_t kSize = 57; // bytes of one TFurnitureRec in the server's list
        static constexpr std::size_t kLinks = 11; // most pieces linked (placed on) to one piece

        int itemId = 0;
        int cellX = 0;
        int cellY = 0;
        int cellZ = 0;
        int rotation = 0;
        int parent = 0;
        std::array<int, kLinks> links{};
        int room = 0;
        int retained = 0;
        int usedCount = 0; // a Magic Sandglass lasts 250

        /// @brief Reads the 57 bytes of a record at the reader's place.
        /// @param reader Reader positioned at the record.
        /// @return The record (default values when the reader runs short).
        static FurnitureRecord read(core::ByteReader& reader);
    };

}

#endif
