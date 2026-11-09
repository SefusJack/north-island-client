#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_WORLD_MAP_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_WORLD_MAP_H

#include "client/models/world/DoorLight.h"
#include "client/models/world/GroundObject.h"
#include "client/models/world/GroundTile.h"
#include "core/Point.h"
#include "formats/DataArchive.h"
#include "formats/MapFile.h"
#include "gfx/PictureId.h"
#include "gfx/PictureLibrary.h"

#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace client {

    /// @brief The map the player is on: its file (size, cells), background picture, ground objects and, for tile maps, its tiles.
    /// @details All its pictures carry the map's tint.
    class WorldMap {
        public:
            /// @brief Loads a map and its objects.
            /// @details Original: the map file comes from Ground.MMG (an instance's runtime map reads the file of the scene it copies), the
            /// objects from Wem.MMG. A background id other than 0 is drawn instead of the file's (the floor room of a house: its picture is by the room's size).
            /// @param mapId The map id.
            /// @param fileId The id of the map file to read.
            /// @param maps Ground.MMG archive.
            /// @param objects Wem.MMG archive.
            /// @param pictures Picture library.
            /// @param backgroundId Background picture id overriding the file's, 0 for none.
            /// @return The map, or nothing when the file cannot be read.
            static std::optional<WorldMap> load(
                int mapId,
                int fileId,
                formats::DataArchive& maps,
                formats::DataArchive& objects,
                gfx::PictureLibrary& pictures,
                int backgroundId = 0
            );

            /// @brief The map id.
            /// @return The id.
            int id() const { return m_id; }

            /// @brief The map file.
            /// @return The parsed file.
            const formats::MapFile& file() const { return m_file; }

            /// @brief The map width.
            /// @return Width in pixels.
            int width() const { return m_file.width(); }

            /// @brief The map height.
            /// @return Height in pixels.
            int height() const { return m_file.height(); }

            /// @brief The background picture.
            /// @return The picture id.
            gfx::PictureId background() const { return m_background; }

            /// @brief Draws another picture as the background (the floor room of a house changed its size).
            /// @param backgroundId New background picture id.
            /// @param pictures Picture library.
            void changeBackground(int backgroundId, gfx::PictureLibrary& pictures);

            /// @brief The ground objects.
            /// @return Mutable list.
            std::vector<GroundObject>& objects() { return m_objects; }

            /// @brief The ground objects.
            /// @return Read-only list.
            const std::vector<GroundObject>& objects() const { return m_objects; }

            /// @brief The tiles of a tile map.
            /// @return Read-only list.
            const std::vector<GroundTile>& tiles() const { return m_tiles; }

            /// @brief The door lights.
            /// @return Mutable list.
            std::vector<DoorLight>& doorLights() { return m_doorLights; }

            /// @brief Adds a door light.
            /// @param light The light.
            void addDoorLight(const DoorLight& light) { m_doorLights.push_back(light); }

            /// @brief The large tile picture ("500A") that maps of tile mode 2 lay on the map points of largeTilePositions().
            /// @return The picture id.
            gfx::PictureId largeTile() const { return m_largeTile; }

            /// @brief The map points where the 480 x 480 large tile picture goes.
            /// @return Read-only list of points.
            const std::vector<core::Point>& largeTilePositions() const { return m_largeTilePositions; }

            /// @brief Gives the attribute of the cell under a map pixel.
            /// @details Original: TMap.GetTileAttr; 1 for walls and beyond the map.
            /// @param mapPixel Map pixel.
            /// @return The attribute.
            int cellAttribute(core::Point mapPixel) const;

            /// @brief Gives the attribute bits of a cell, with the bit of a cell a standing NPC takes (4).
            /// @details Original: FTileAttrs of a cell (0-based).
            /// @param cell Cell coordinates (0-based).
            /// @return The attribute bits.
            int tileAttribute(core::Point cell) const;

            /// @brief Tells whether someone may stand on a map pixel: on foot (class 0) or on a vehicle of class 1 (ground), 2 (water) or 3.
            /// @details Original: WalkBlockMasks + 4; a boat only goes over water.
            /// @param mapPixel Map pixel.
            /// @param walkClass Walk class 0..3.
            /// @return True when walkable.
            bool isWalkable(core::Point mapPixel, int walkClass = 0) const;

            /// @brief A standing NPC takes the cell under its feet and the one above.
            /// @details Original: BlockNpcCells.
            /// @param mapPixel The NPC's map pixel.
            void occupy(core::Point mapPixel);

        private:
            /// @brief Builds the ground tiles of a tile map (or the large tile positions).
            /// @param pictures Picture library.
            void placeTiles(gfx::PictureLibrary& pictures);

            int m_id = 0;
            formats::MapFile m_file;
            gfx::PictureId m_background;
            std::vector<GroundObject> m_objects;
            std::vector<GroundTile> m_tiles;
            std::vector<DoorLight> m_doorLights;
            gfx::PictureId m_largeTile;
            std::vector<core::Point> m_largeTilePositions;
            std::set<std::pair<int, int>> m_occupied;
    };

}

#endif
