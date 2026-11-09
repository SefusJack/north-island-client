#include "client/models/world/WorldMap.h"

#include "core/Log.h"

#include <map>
#include <string>

namespace client {

    namespace {

        // cell attribute bits that block walking, indexed by walk class 0..3
        constexpr std::uint8_t kBlocks[4] = {
            formats::MapFile::kWall | formats::MapFile::kDeepWater | formats::MapFile::kOccupied | formats::MapFile::kBlocked,
            formats::MapFile::kWall | formats::MapFile::kDeepWater | formats::MapFile::kOccupied | formats::MapFile::kShallowWater | formats::MapFile::kBlocked,
            formats::MapFile::kWall | formats::MapFile::kOccupied | formats::MapFile::kBlocked,
            formats::MapFile::kOccupied | formats::MapFile::kBlocked
        };
        constexpr int kBoatClass = 2; // walk class 2: boat (may cross water; a cell with attribute 0 is not walkable for it)

    }

    void WorldMap::changeBackground(int backgroundId, gfx::PictureLibrary& pictures) {
        m_background = pictures.find(std::to_string(backgroundId), m_file.tint()).value_or(gfx::PictureId{});
    }

    std::optional<WorldMap> WorldMap::load(
        int mapId,
        int fileId,
        formats::DataArchive& maps,
        formats::DataArchive& objects,
        gfx::PictureLibrary& pictures,
        int backgroundId
    ) {
        const auto data = maps.read(std::to_string(fileId) + ".map");
        auto file = data ? formats::MapFile::parse(*data) : std::nullopt;
        if (!file) {
            core::logError("cannot read map ", mapId);
            return std::nullopt;
        }
        WorldMap map;
        map.m_id = mapId;
        map.m_file = std::move(*file);
        const core::Color tint = map.m_file.tint();
        map.m_background = pictures.find(std::to_string(backgroundId != 0 ? backgroundId : map.m_file.backgroundId()), tint).value_or(gfx::PictureId{});

        std::map<std::uint32_t, std::optional<formats::GroundObjectInfo>> infos;
        for (const formats::MapObjectPlacement& placement : map.m_file.objects()) {
            auto found = infos.find(placement.objectId);
            if (found == infos.end()) {
                const auto record = objects.read(std::to_string(placement.objectId) + ".wem");
                found = infos.emplace(placement.objectId, record ? formats::GroundObjectInfo::parse(*record) : std::nullopt).first;
            }
            if (!found->second) {
                core::logWarning("map ", mapId, ": no ground object ", placement.objectId);
                continue;
            }
            GroundObject object;
            object.info = *found->second;
            object.picture = pictures.find(std::to_string(object.info.pictureId), tint).value_or(gfx::PictureId{});
            object.light = object.picture.isValid() &&
                           pictures.picture(object.picture).topLeft().toRgb565() == 1; // a top-left pixel of RGB565 value 1 marks a light (glow) object
            object.position = placement.position;
            object.frame = object.info.firstFrame % object.info.frames;
            map.m_objects.push_back(object);
        }
        map.placeTiles(pictures);
        core::logInfo("map ", mapId, ": ", map.width(), 'x', map.height(), ", ", map.m_objects.size(), " objects");
        return map;
    }

    void WorldMap::placeTiles(gfx::PictureLibrary& pictures) {
        const core::Color tint = m_file.tint();
        const auto picture = [&](int number, int variant, int variants) {
            if (variant < 1 || variant > variants) {
                return pictures.find(std::to_string(number), tint).value_or(gfx::PictureId{});
            }
            return pictures.find(std::to_string(number) + static_cast<char>('A' + variant - 1), tint).value_or(gfx::PictureId{});
        };
        if (m_file.tileMode() == 2) { // tile mode 2: large 480 x 480 pictures
            m_largeTile = pictures.find("500A", tint).value_or(gfx::PictureId{}); // picture 500A: the large ground tile
            for (int row = 1; row <= m_file.tileRows(); ++row) {
                for (int column = 1; column <= m_file.tileColumns(); ++column) {
                    if (m_file.tile(row, column).ground != 0) {
                        m_largeTilePositions.push_back({(row - 1) * 480, (column - 1) * 480}); // each large tile is 480 x 480 map pixels
                    }
                }
            }
            return;
        }
        if (m_file.tileMode() != 1) {
            return;
        }
        const int columns = m_file.tileColumns();
        for (int row = 1; row <= m_file.tileRows(); ++row) {
            for (int column = 1; column <= columns; ++column) {
                const formats::MapTile& source = m_file.tile(row, column);
                const int rowShift = row == 1 ? 0 : row - 1;
                const int columnShift = column == 1 ? 0 : column - 1;
                // isometric tile: 181 px per half-step in x, 91 px in y; later rows/columns overlap their neighbour by 1 px
                const core::Point origin{
                    columns * 181 + (row - 1) * 181 - (column - 1) * 181 - 181 - rowShift + columnShift,
                    (row - 1) * 91 + (column - 1) * 91 - rowShift - columnShift
                };
                GroundTile tile;
                tile.row = row;
                tile.column = column;
                tile.groundNumber = source.ground;
                tile.objectBase = source.objectBase;
                if (source.groundVariant != 0) {
                    tile.ground = picture(source.ground, source.groundVariant, 6); // ground pictures have up to 6 variants (A..F)
                    tile.groundPosition = {origin.x, origin.y + 1};
                }
                if (source.objectBase != 0) {
                    const int number = source.objectBase + source.objectNumber;
                    tile.object = picture(number, source.objectVariant, 4); // object pictures have up to 4 variants (A..D)
                    // object number 17 is not lifted, others 150 px; tile set base 10100 lifts 3 px more
                    const int lift = (source.objectNumber == 17 ? 0 : 150) + (source.objectBase == 10100 ? 3 : 0);
                    // object is drawn 3 + 4 px below the tile origin, less the lift
                    tile.objectPosition = {origin.x, origin.y + 3 + 4 - lift};
                    tile.sortY = source.sortKey;
                    // object numbers 13, 14 in variants 2, 3 of tile set bases 10000, 11000, 60000 have a back part
                    const bool hasBack = (source.objectNumber == 13 || source.objectNumber == 14) && (source.objectVariant == 2 || source.objectVariant == 3) &&
                                         (source.objectBase == 10000 || source.objectBase == 11000 || source.objectBase == 60000);
                    if (hasBack && source.groundVariant != 0) {
                        const std::string name = std::to_string(number) + static_cast<char>('A' + source.objectVariant - 1) + "_B";
                        tile.objectBack = pictures.find(name, tint).value_or(gfx::PictureId{});
                        tile.objectBackPosition = {origin.x, origin.y - 150 + 1 + 3}; // back part sits 150 px up, nudged 1 + 3 px down
                    }
                }
                if (tile.ground.isValid() || tile.object.isValid()) {
                    m_tiles.push_back(tile);
                }
            }
        }
    }

    int WorldMap::cellAttribute(core::Point mapPixel) const {
        const int cell = formats::MapFile::kCellSize;
        return m_file.attribute((mapPixel.x + cell - 1) / cell - 1, (mapPixel.y + cell - 1) / cell - 1);
    }

    int WorldMap::tileAttribute(core::Point cell) const {
        return m_file.attribute(cell.x, cell.y) | (m_occupied.count({cell.x, cell.y}) != 0 ? formats::MapFile::kOccupied : 0);
    }

    bool WorldMap::isWalkable(core::Point mapPixel, int walkClass) const {
        if (mapPixel.x < 0 || mapPixel.y < 0 || walkClass < 0 || walkClass > 3) { // walk classes are 0..3
            return false;
        }
        const int cell = formats::MapFile::kCellSize;
        const int value = tileAttribute({mapPixel.x / cell, mapPixel.y / cell});
        return (value & kBlocks[walkClass]) == 0 && !(walkClass == kBoatClass && value == 0);
    }

    void WorldMap::occupy(core::Point mapPixel) {
        const int cell = formats::MapFile::kCellSize;
        m_occupied.insert({mapPixel.x / cell, mapPixel.y / cell});
        m_occupied.insert({mapPixel.x / cell, mapPixel.y / cell - 1});
    }

}
