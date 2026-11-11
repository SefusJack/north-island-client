#include "formats/EventArchive.h"

#include "core/ByteReader.h"
#include "core/FileSystem.h"

#include <algorithm>
#include <cstdlib>
#include <set>

namespace formats {

    namespace {

        constexpr std::size_t kHeaderSize = 12; // file header: f64 (8) + i32 entry count (4)
        constexpr std::size_t kEntrySize = 10; // index entry: u16 map id + u16 source map + u32 offset + u16 size
        constexpr std::uint16_t kVersion = 112; // scene blob format version
        constexpr int kFallbackMap = 12003; // map 12003: Civillian's House; its scene is used for maps without one
        constexpr std::size_t kNameSize = 20; // bytes of a name field (1 length byte + 19 characters)
        constexpr std::size_t kNpcItemSize = 92; // bytes of one event-walk block in an NPC record
        constexpr int kClickFlag = 1; // event flag value 1 (TEvent.FFlag0): a door with such an event starts it on a click inside its area
        constexpr int kNearFlag = 2; // event flag value 2 (TEvent.FFlag1): a "near" event, listed in EventScene::nearEvents
        constexpr int kMostWalkPoints = 10; // most points in one event walk
        constexpr int kCell = 20; // map cell size, px
        constexpr std::size_t kConditionSize = 22; // bytes of an event condition record
        constexpr std::size_t kResultSize = 22; // bytes of an event result record
        constexpr std::size_t kGroupMemberSize = 3; // bytes of one group member entry
        constexpr std::size_t kConveySize = 33; // bytes of a warp (convey) record after its 2-byte index

        /// @brief Reads a u8 count followed by that many u8 values.
        /// @param reader Reader positioned at the count.
        /// @return The values.
        std::vector<int> byteList(core::ByteReader& reader) {
            const int count = reader.u8();
            std::vector<int> values;
            for (int i = 0; i < count; ++i) {
                values.push_back(reader.u8());
            }
            return values;
        }

        /// @brief The start every placed record shares: name, flags, position, event ids and the minimap sign.
        struct PlacedRecord {
            std::string name;
            int flags = 0;
            core::Point position;
            std::vector<int> eventIds;
            int sign = 0;
        };

        /// @brief Reads the start every placed record shares.
        /// @param reader Reader positioned at the record.
        /// @return The common fields.
        /// @details Original: sub_004CC548. The byte after the two lists (B3C) is the object's minimap sign (Icon_Sign_<n>; 0 none), which the client copies to the NPC's FObjState and the door's / mine's byte $18 on every map load (RE_EveStruct.pas:795-823, BK_EventManage.pas:809, 2751).
        PlacedRecord readPlaced(core::ByteReader& reader) {
            PlacedRecord record;
            const std::size_t length = std::min<std::size_t>(reader.u8(), kNameSize - 1);
            record.name = reader.text(kNameSize - 1).substr(0, length);
            record.flags = reader.u8();
            record.position.x = reader.i32();
            record.position.y = reader.i32();
            record.eventIds = byteList(reader);
            byteList(reader);
            record.sign = reader.u8();
            return record;
        }

        /// @brief Reads one NPC record: placement, move path, trigger area and event walks.
        /// @param reader Reader positioned at the record.
        /// @param index The NPC's index in the scene.
        /// @return The NPC.
        SceneNpc readNpc(core::ByteReader& reader, int index) {
            const PlacedRecord placed = readPlaced(reader);
            SceneNpc npc;
            npc.index = index;
            npc.name = placed.name;
            npc.flags = placed.flags;
            npc.position = placed.position;
            npc.mapSign = placed.sign;
            npc.npcId = reader.i32();
            npc.direction = reader.u8();
            npc.moveKind = reader.u8();
            npc.pathEnd = reader.u8();
            const int points = reader.u8();
            for (int i = 0; i < points; ++i) {
                SceneNpc::PathPoint point;
                point.position.x = reader.i32();
                point.position.y = reader.i32();
                point.value = reader.i32();
                npc.path.push_back(point);
            }
            npc.speed = reader.u8();
            reader.skip(1);
            npc.triggerHalfSize.x = reader.i32();
            npc.triggerHalfSize.y = reader.i32();
            if (npc.moveKind != 4 && npc.moveKind != 5) { // move kinds 4 and 5 keep the trigger area (half size in cells)
                npc.triggerHalfSize = {};
            }
            npc.eventIds = placed.eventIds;
            reader.skip(2 + 1); // 3 bytes not used by the client
            const int walks = reader.u8();
            for (int i = 0; i < walks; ++i) {
                const std::size_t start = reader.position();
                const int number = reader.u8();
                const int used = reader.u8();
                reader.skip(1);
                const int count = std::min(static_cast<int>(reader.u8()), kMostWalkPoints);
                reader.skip(8); // 8 bytes before the points (the first of eleven places)
                std::vector<core::Point> walk;
                for (int k = 0; k < count; ++k) {
                    const int x = reader.i32();
                    const int y = reader.i32();
                    walk.push_back({x, y});
                }
                if (used != 0 && !walk.empty()) {
                    npc.eventWalks[number] = std::move(walk);
                }
                reader.seek(start + kNpcItemSize);
            }
            reader.skip(2 + 2); // 4 bytes not used by the client
            npc.drawOffsetY = reader.i32();
            return npc;
        }

        /// @brief Reads one door record; door cells are counted from 1 and the area covers both corner cells.
        /// @param reader Reader positioned at the record.
        /// @param index The door's index in the scene.
        /// @return The door.
        SceneDoor readDoor(core::ByteReader& reader, int index) {
            const PlacedRecord placed = readPlaced(reader);
            SceneDoor door;
            door.index = index;
            door.name = placed.name;
            door.flags = placed.flags;
            door.mapSign = placed.sign;
            door.eventIds = placed.eventIds;
            const int x2 = reader.i32();
            const int y2 = reader.i32();
            door.kind = reader.u8();
            const int x = placed.position.x;
            const int y = placed.position.y;
            door.lightPosition.x = reader.i32() + (x - 1) * kCell;
            door.lightPosition.y = reader.i32() + (y - 1) * kCell;
            door.area = {(std::min(x, x2) - 1) * kCell, (std::min(y, y2) - 1) * kCell, (std::abs(x2 - x) + 1) * kCell, (std::abs(y2 - y) + 1) * kCell};
            return door;
        }

        /// @brief Passes over ReadClsFmt's sections between the doors and the interacts (mines, goods, events, groups, warps), keeping only the events' near flags.
        /// @param reader Reader positioned after the doors.
        /// @param scene Scene that receives the near events.
        void readToSurfaces(core::ByteReader& reader, EventScene& scene) {
            const int mines = reader.u16();
            for (int i = 0; i < mines && !reader.failed(); ++i) {
                // mine: 2 bytes (index), the placed record, then 10 bytes not used
                reader.skip(2);
                readPlaced(reader);
                reader.skip(4 + 4 + 1 + 1);
            }
            const int goods = reader.u16();
            for (int i = 0; i < goods && !reader.failed(); ++i) {
                // goods: 2 bytes (index), the placed record, then 11 bytes not used
                reader.skip(2);
                readPlaced(reader);
                reader.skip(4 + 2 + 1 + 4);
            }
            const int events = reader.u16();
            std::set<int> clickEvents;
            for (int i = 0; i < events && !reader.failed(); ++i) {
                const int id = reader.u16();
                const int flags = reader.u8();
                if ((flags & kNearFlag) != 0) {
                    scene.nearEvents.insert(id);
                }
                if ((flags & kClickFlag) != 0) {
                    clickEvents.insert(id);
                }
                reader.skip(kNameSize);
                const int conditions = reader.u8();
                for (int c = 0; c < conditions && !reader.failed(); ++c) {
                    reader.skip(kConditionSize);
                    reader.skip(reader.u8() * kResultSize);
                }
            }
            for (SceneDoor& door : scene.doors) {
                door.clickable = std::any_of(door.eventIds.begin(), door.eventIds.end(), [&clickEvents](int id) { return clickEvents.count(id) != 0; });
            }
            const int groups = reader.u16();
            for (int i = 0; i < groups && !reader.failed(); ++i) {
                reader.skip(2 + kNameSize + 1 + 2 + 1 + 1); // group header: index (2), name, then 5 bytes not used
                reader.skip(reader.u16() * kGroupMemberSize);
            }
            const int conveys = reader.u16();
            for (int i = 0; i < conveys && !reader.failed(); ++i) {
                reader.skip(2 + kConveySize); // warp: index (2) and its record
            }
        }

        /// @brief Reads one interact record (name, kind, value and its entries of 5 bytes each).
        /// @param reader Reader positioned at the record.
        /// @param index The surface's index in the scene.
        /// @return The surface.
        SceneSurface readSurface(core::ByteReader& reader, int index) {
            SceneSurface surface;
            surface.index = index;
            reader.skip(kNameSize);
            surface.kind = reader.u8();
            surface.value = reader.u16();
            const int entries = reader.u8();
            for (int i = 0; i < entries; ++i) {
                SceneSurface::Entry entry;
                entry.number = reader.u8();
                entry.id = reader.u16();
                entry.source = reader.u8();
                entry.kind = reader.u8();
                surface.entries.push_back(entry);
            }
            return surface;
        }

    }

    std::optional<EventArchive> EventArchive::open(const std::filesystem::path& path) {
        auto data = core::readFile(path);
        if (!data || data->size() < kHeaderSize) {
            return std::nullopt;
        }
        EventArchive archive;
        archive.m_data = std::move(*data);
        core::ByteReader reader(archive.m_data, 8); // skip the leading f64 (8 bytes)
        const std::int32_t count = reader.i32();
        if (count <= 0 || kHeaderSize + static_cast<std::size_t>(count) * kEntrySize > archive.m_data.size()) {
            return std::nullopt;
        }
        for (std::int32_t i = 0; i < count; ++i) {
            const int mapId = reader.u16();
            reader.skip(2); // source map (u16), not used
            Entry entry;
            entry.offset = reader.u32();
            entry.size = reader.u16();
            if (entry.offset + entry.size <= archive.m_data.size()) {
                archive.m_entries.emplace(mapId, entry);
            }
        }
        return archive;
    }

    std::optional<EventScene> EventArchive::scene(int mapId) const {
        auto found = m_entries.find(mapId);
        if (found == m_entries.end()) {
            found = m_entries.find(kFallbackMap);
        }
        if (found == m_entries.end()) {
            return std::nullopt;
        }
        const core::Bytes blob(
            m_data.begin() + static_cast<std::ptrdiff_t>(found->second.offset),
            m_data.begin() + static_cast<std::ptrdiff_t>(found->second.offset + found->second.size)
        );
        core::ByteReader reader(blob);
        reader.skip(2); // the scene's map id (u16)
        if (reader.u16() != kVersion) {
            return std::nullopt;
        }
        EventScene scene;
        const int npcs = reader.u16();
        for (int i = 0; i < npcs; ++i) {
            const int index = reader.u16();
            scene.npcs.push_back(readNpc(reader, index));
        }
        const int doors = reader.u16();
        for (int i = 0; i < doors; ++i) {
            const int index = reader.u16();
            scene.doors.push_back(readDoor(reader, index));
        }
        if (reader.failed()) {
            return std::nullopt;
        }
        readToSurfaces(reader, scene);
        const int surfaces = reader.failed() ? 0 : reader.u16();
        for (int i = 0; i < surfaces && !reader.failed(); ++i) {
            const int index = reader.u16();
            SceneSurface surface = readSurface(reader, index);
            if (!reader.failed()) {
                scene.surfaces.emplace(index, std::move(surface));
            }
        }
        constexpr std::size_t kMemberSize = 3;
        const int battles = reader.failed() ? 0 : reader.u16();
        for (int i = 0; i < battles && !reader.failed(); ++i) {
            const int id = reader.u16();
            readPlaced(reader);
            const int music = reader.u8();
            reader.skip(2 + 1); // 3 bytes not used by the client
            reader.skip(reader.u8() * kMemberSize);
            reader.skip(reader.u8() * kMemberSize);
            reader.skip(1);
            if (!reader.failed()) {
                scene.battleMusic[id] = music;
            }
        }
        return scene;
    }

}
