#ifndef NORTH_ISLAND_FORMATS_NPC_TABLE_H
#define NORTH_ISLAND_FORMATS_NPC_TABLE_H

#include "core/Bytes.h"
#include "formats/NpcInfo.h"

#include <map>
#include <optional>

namespace formats {

    /// @brief The NPC table loaded from Data/Npc.dat.
    /// @details Original: TFNpc; 148-byte records after a header record. Bytes are stored (v xor 200) - 1, words (v xor 21001) - 1,
    /// integers (v xor 195999510) - 1; the name is a string[20] with its characters reversed. Offsets: 0 name, 21 type, 22 NPC id, 24 sprite,
    /// 26 role sprite, 28..43 four colour codes, 48 HP of a bar, 52 collection number, 56..65 the five values, 67 element, 68..73 three skills,
    /// 84 shadow, 93 direction kind, 96 weapon, 97 sprite size, 100 monster book key, 109 grow key, 120 mercenary.
    class NpcTable {
        public:
            /// @brief Parses Npc.dat.
            /// @param data Raw file bytes.
            /// @return The table, or nothing when the file holds less than two records.
            static std::optional<NpcTable> parse(const core::Bytes& data);

            /// @brief Looks an NPC kind up by id.
            /// @param npcId NPC id.
            /// @return The kind, or nothing for unknown ids.
            std::optional<NpcInfo> find(int npcId) const;

            /// @brief Finds the kind with a monster book key.
            /// @details Original: GetNpcIndexByKey.
            /// @param npcKey Monster book key.
            /// @return The kind, or nothing.
            std::optional<NpcInfo> findByKey(int npcKey) const;

            /// @brief Finds the kind with a grow key.
            /// @details Original: GetNpcIdByGrowKey (grow keys are 1..50).
            /// @param growKey Grow key.
            /// @return The kind, or nothing.
            std::optional<NpcInfo> findByGrowKey(int growKey) const;

            /// @brief Replaces a kind's name from a text pack (the qol client's mods).
            /// @param npcId NPC id.
            /// @param name New name.
            /// @return True when the kind exists.
            bool setName(int npcId, const std::string& name);

        private:
            std::map<int, NpcInfo> m_npcs;
            std::map<int, int> m_keys;
            std::map<int, int> m_growKeys;
    };

}

#endif
