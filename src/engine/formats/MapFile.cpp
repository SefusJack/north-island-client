#include "formats/MapFile.h"

#include "core/ByteReader.h"

namespace formats {

    namespace {

        constexpr std::size_t kSoundAreaSize = 6; // bytes per sound area record
        constexpr std::size_t kBattleZoneSize = 9; // bytes per battle zone record
        constexpr std::size_t kUnknownBlockSize = 10 + 4 + 4; // bytes of an unread block between the battle zones and the tint

    }

    std::optional<MapFile> MapFile::parse(const core::Bytes& data) {
        core::ByteReader reader(data);
        MapFile map;
        map.m_width = reader.i32();
        map.m_height = reader.i32();

        const int backgrounds = reader.u8();
        for (int i = 0; i < backgrounds; ++i) {
            map.m_backgroundId = reader.u16();
            const int x = reader.u16();
            const int y = reader.u16();
            map.m_backgroundPosition = backgrounds == 1 ? core::Point{0, 0} : core::Point{x, y};
        }

        map.m_cellsX = reader.u16();
        map.m_cellsY = reader.u16();
        const core::Bytes attributes = reader.bytes(static_cast<std::size_t>(map.m_cellsX) * static_cast<std::size_t>(map.m_cellsY));
        map.m_attributes.assign(attributes.begin(), attributes.end());

        reader.skip(reader.u16() * kSoundAreaSize);

        const int objects = reader.u16();
        for (int i = 0; i < objects; ++i) {
            MapObjectPlacement object;
            object.objectId = reader.u32();
            object.position.x = static_cast<std::int16_t>(reader.u16());
            object.position.y = static_cast<std::int16_t>(reader.u16());
            map.m_objects.push_back(object);
        }

        reader.skip(2); // two unread bytes before the battle zone count
        reader.skip(reader.u8() * kBattleZoneSize);
        reader.skip(kUnknownBlockSize);
        map.m_tint.r = reader.u8();
        map.m_tint.g = reader.u8();
        map.m_tint.b = reader.u8();
        if (!reader.atEnd()) {
            map.m_tileMode = reader.u8();
            map.m_tileRows = reader.u8();
            map.m_tileColumns = reader.u8();
            for (int i = 0; i < map.m_tileRows * map.m_tileColumns; ++i) {
                MapTile tile;
                tile.ground = reader.u16();
                tile.objectBase = reader.u16();
                tile.groundVariant = reader.u8();
                tile.objectNumber = reader.u8();
                tile.objectVariant = reader.u8();
                tile.sortKey = reader.i32();
                map.m_tiles.push_back(tile);
            }
        }
        if (reader.failed() || map.m_width <= 0 || map.m_height <= 0) {
            return std::nullopt;
        }
        return map;
    }

    std::uint8_t MapFile::attribute(int cellX, int cellY) const {
        if (cellX < 0 || cellY < 0 || cellX >= m_cellsX || cellY >= m_cellsY) {
            return kWall;
        }
        return m_attributes[static_cast<std::size_t>(cellX) * static_cast<std::size_t>(m_cellsY) + static_cast<std::size_t>(cellY)];
    }

}
