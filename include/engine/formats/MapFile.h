#ifndef NORTH_ISLAND_FORMATS_MAP_FILE_H
#define NORTH_ISLAND_FORMATS_MAP_FILE_H

#include "core/Bytes.h"
#include "core/Color.h"
#include "core/Point.h"
#include "formats/MapObjectPlacement.h"
#include "formats/MapTile.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace formats {

    /// @brief A map file: size, background, cell attributes, ground objects, tint and optional tiles.
    /// @details Original: Ground.MMG "<mapId>.map", TGameMap.LoadMapFile. Little-endian layout:
    /// i32 width, i32 height (pixels); u8 n, n x (u16 jpeg id, u16 x, u16 y) background (the game keeps only the last one);
    /// u16 cellsX, u16 cellsY, cellsX x cellsY attribute bytes column by column; u16 n, n x 6 bytes ambient sound areas (not read yet);
    /// u16 n, n x (u32 object id, i16 x, i16 y) ground objects; u8, u8, u8 n, n x 9 bytes battle zones (not read);
    /// 10 bytes, i32, i32, u8 r, g, b the tint added to the map's pictures; optionally u8 tile mode (1 isometric tiles, 2 large tiles),
    /// u8 rows, u8 columns, then per row and column u16 ground, u16 object base, u8 ground variant, u8 object number, u8 object variant, i32 sort row (MapTile).
    /// Cells are 20 x 20 pixels.
    class MapFile {
        public:
            static constexpr int kCellSize = 20; // attribute cell width and height, px

            static constexpr std::uint8_t kWall = 1;
            static constexpr std::uint8_t kDeepWater = 2; // cell attribute bit 2: deep water
            static constexpr std::uint8_t kOccupied = 4; // cell attribute bit 4: occupied
            static constexpr std::uint8_t kShallowWater = 8; // cell attribute bit 8: shallow water
            static constexpr std::uint8_t kBlocked = 16; // cell attribute bit 16: blocked

            /// @brief Parses a .map file.
            /// @param data Raw file bytes.
            /// @return The map, or nothing when the data is truncated or the size is not positive.
            static std::optional<MapFile> parse(const core::Bytes& data);

            /// @brief Map width in pixels.
            /// @return The width.
            int width() const { return m_width; }

            /// @brief Map height in pixels.
            /// @return The height.
            int height() const { return m_height; }

            /// @brief Picture id of the (last) background.
            /// @return The JPEG id.
            int backgroundId() const { return m_backgroundId; }

            /// @brief Where the background goes (0,0 when the map has a single one).
            /// @return The position in map pixels.
            core::Point backgroundPosition() const { return m_backgroundPosition; }

            /// @brief The attribute bits of a cell; cells outside the map count as walls.
            /// @param cellX Cell column.
            /// @param cellY Cell row.
            /// @return The attribute bits (kWall, kDeepWater, ...).
            std::uint8_t attribute(int cellX, int cellY) const;

            /// @brief The ground objects placed on the map.
            /// @return The placements.
            const std::vector<MapObjectPlacement>& objects() const { return m_objects; }

            /// @brief The tint added to the map's pictures.
            /// @return The tint colour.
            core::Color tint() const { return m_tint; }

            /// @brief Tile mode: 0 none, 1 isometric tiles, 2 large tiles.
            /// @return The mode.
            int tileMode() const { return m_tileMode; }

            /// @brief Number of tile rows.
            /// @return The row count.
            int tileRows() const { return m_tileRows; }

            /// @brief Number of tile columns.
            /// @return The column count.
            int tileColumns() const { return m_tileColumns; }

            /// @brief One tile of the map.
            /// @param row Row, 1..rows.
            /// @param column Column, 1..columns.
            /// @return The tile.
            const MapTile& tile(int row, int column) const { return m_tiles[static_cast<std::size_t>((row - 1) * m_tileColumns + (column - 1))]; }

        private:
            int m_width = 0;
            int m_height = 0;
            int m_backgroundId = 0;
            core::Point m_backgroundPosition;
            int m_cellsX = 0;
            int m_cellsY = 0;
            std::vector<std::uint8_t> m_attributes;
            std::vector<MapObjectPlacement> m_objects;
            core::Color m_tint{0, 0, 0, 0};
            int m_tileMode = 0;
            int m_tileRows = 0;
            int m_tileColumns = 0;
            std::vector<MapTile> m_tiles;
    };

}

#endif
