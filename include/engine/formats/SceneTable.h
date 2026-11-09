#ifndef NORTH_ISLAND_FORMATS_SCENE_TABLE_H
#define NORTH_ISLAND_FORMATS_SCENE_TABLE_H

#include "core/Bytes.h"
#include "formats/SceneInfo.h"

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace formats {

    /// @brief The scene table loaded from Data/SceneData.Dat.
    /// @details Original: TFSceneData, DecodeSceneData; 131-byte records, record 0 a header whose first word is 4. Per record: u16 id at 0,
    /// string[20] name at 2 (characters reversed), u8 type at 23, string[7] music at 25 (characters reversed), u8 flags at 33, u16 pictures at 80
    /// (four: the second to fourth are a battle ground's layers), u16 layer offsets at 88 and 90, the instance fields at 96 (players), 97 (guilds only),
    /// 101 (mark), 103 (level), 105 (minutes). Words are stored (v xor 60012) - 9, bytes (v xor 44) - 9. Like DecodeSceneData it makes ten runtime maps
    /// (61501 + ...) for each instance scene: the scenes with players above 0, in file order, at most 25, except 30005.
    class SceneTable {
        public:
            static constexpr int kFirstInstanceMap = 61501; // first runtime instance map id (map 61501)
            static constexpr int kMapsPerInstance = 10; // runtime maps made per instance scene

            /// @brief Parses SceneData.Dat.
            /// @param data Raw file bytes.
            /// @return The table, or nothing when the header is missing or its version is not 4.
            static std::optional<SceneTable> parse(const core::Bytes& data);

            /// @brief Looks a scene up by map id (an instance map gives the scene it copies).
            /// @param mapId Map id.
            /// @return The scene, or nothing for unknown maps.
            std::optional<SceneInfo> find(int mapId) const;

            /// @brief The ids of the instance scenes.
            /// @details Original: AddDupMission.
            /// @return The scene ids in file order.
            const std::vector<int>& instanceScenes() const { return m_instances; }

            /// @brief Whether a map id is a runtime map of an instance (the last bound counts too, as in the game).
            /// @details Original: IsDupMissionMap.
            /// @param mapId Map id.
            /// @return True for an instance map.
            bool isInstanceMap(int mapId) const;

            /// @brief The scene a runtime instance map copies.
            /// @details Original: GetDupMissionForMap.
            /// @param mapId Map id.
            /// @return The scene id, or 0 for none.
            int instanceSceneOf(int mapId) const;

        private:
            std::map<std::uint16_t, SceneInfo> m_scenes;
            std::vector<int> m_instances;
    };

}

#endif
