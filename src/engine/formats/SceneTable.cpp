#include "formats/SceneTable.h"

#include "core/ByteReader.h"
#include "core/StringUtil.h"

#include <algorithm>

namespace formats {

    namespace {

        constexpr std::size_t kRecordSize = 131; // bytes per SceneData.Dat record
        constexpr std::uint16_t kVersion = 4; // first word of the header record
        constexpr std::size_t kNameAt = 2; // record offset of the scene name (length byte first)
        constexpr std::size_t kNameLength = 20; // bytes of the name field (after its length byte)
        constexpr std::size_t kTypeAt = 23; // record offset of the scene type
        constexpr std::size_t kMusicAt = 25; // record offset of the music name (length byte first)
        constexpr std::size_t kMusicLength = 7; // bytes of the music name field (after its length byte)
        constexpr std::size_t kFlagsAt = 33; // record offset of the scene flags
        constexpr std::size_t kEffectAt = 35; // record offset of the scene effect
        constexpr std::size_t kBattlePicturesAt = 82; // record offset of the battle ground pictures (3 words)
        constexpr std::size_t kBattleLayerYAt = 88; // record offset of the battle layer y offsets (2 words)
        constexpr std::size_t kTitleLightAt = 94; // record offset of the title light
        constexpr std::size_t kDupMaxPlayersAt = 96; // record offset of the instance max players (then guild-only flag at 97)
        constexpr std::size_t kDupMarkAt = 101; // record offset of the instance mark number (then min level at 103, time limit at 105)
        constexpr std::uint16_t kNoInstanceScene = 30005; // scene 30005 is never an instance, even with players above 0
        constexpr std::size_t kMostInstances = 25; // most instance scenes (the original's array size)

        /// @brief Decodes a stored word: (v xor 60012) - 9.
        /// @param value Stored value.
        /// @return The real value.
        std::uint16_t decodeWord(std::uint16_t value) { return static_cast<std::uint16_t>((value ^ 60012) - 9); } // 60012: word key; 9: bias

        /// @brief Decodes a stored byte: (v xor 44) - 9.
        /// @param value Stored value.
        /// @return The real value.
        std::uint8_t decodeByte(std::uint8_t value) { return static_cast<std::uint8_t>((value ^ 44) - 9); } // 44: byte key; 9: bias

        /// @brief Reads a length-prefixed short string stored back to front.
        /// @param reader Source positioned at the length byte.
        /// @param capacity Size of the text field in bytes.
        /// @return The trimmed text in reading order.
        std::string reversedText(core::ByteReader& reader, std::size_t capacity) {
            const std::size_t length = std::min<std::size_t>(reader.u8(), capacity);
            std::string text = reader.text(capacity);
            std::reverse(text.begin(), text.end());
            return core::trim(text.substr(0, length));
        }

    }

    std::optional<SceneTable> SceneTable::parse(const core::Bytes& data) {
        core::ByteReader header(data);
        if (data.size() < kRecordSize || header.u16() != kVersion) {
            return std::nullopt;
        }
        SceneTable table;
        for (std::size_t at = kRecordSize; at + kRecordSize <= data.size(); at += kRecordSize) {
            core::ByteReader reader(data, at);
            const std::uint16_t id = decodeWord(reader.u16());
            if (id == 0) {
                continue;
            }
            SceneInfo scene;
            reader.seek(at + kNameAt);
            scene.name = reversedText(reader, kNameLength);
            reader.seek(at + kTypeAt);
            scene.type = decodeByte(reader.u8());
            reader.seek(at + kMusicAt);
            scene.music = reversedText(reader, kMusicLength);
            reader.seek(at + kFlagsAt);
            scene.flags = decodeByte(reader.u8());
            reader.seek(at + kEffectAt);
            scene.effect = decodeByte(reader.u8());
            reader.seek(at + kBattlePicturesAt);
            for (int& picture : scene.battlePictures) {
                picture = decodeWord(reader.u16());
            }
            reader.seek(at + kBattleLayerYAt);
            for (int& y : scene.battleLayerY) {
                y = decodeWord(reader.u16());
            }
            reader.seek(at + kTitleLightAt);
            scene.titleLight = decodeWord(reader.u16());
            reader.seek(at + kDupMaxPlayersAt);
            scene.dupMaxPlayers = decodeByte(reader.u8());
            scene.dupGuildOnly = decodeByte(reader.u8());
            reader.seek(at + kDupMarkAt);
            scene.dupMarkNo = decodeWord(reader.u16());
            scene.dupMinLevel = decodeWord(reader.u16());
            scene.dupTimeLimit = decodeWord(reader.u16());
            if (id == kNoInstanceScene) {
                scene.dupMaxPlayers = 0;
            }
            if (scene.dupMaxPlayers != 0 && table.m_instances.size() < kMostInstances) {
                table.m_instances.push_back(id);
            }
            table.m_scenes.emplace(id, scene);
        }
        return table;
    }

    std::optional<SceneInfo> SceneTable::find(int mapId) const {
        const int source = instanceSceneOf(mapId);
        const auto found = m_scenes.find(static_cast<std::uint16_t>(source != 0 ? source : mapId));
        if (found == m_scenes.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    bool SceneTable::isInstanceMap(int mapId) const {
        return mapId >= kFirstInstanceMap && mapId <= static_cast<int>(m_instances.size()) * kMapsPerInstance + kFirstInstanceMap;
    }

    int SceneTable::instanceSceneOf(int mapId) const {
        if (!isInstanceMap(mapId)) {
            return 0;
        }
        const std::size_t index = static_cast<std::size_t>((mapId - kFirstInstanceMap) / kMapsPerInstance);
        return index < m_instances.size() ? m_instances[index] : 0;
    }

}
