#include "formats/NpcTable.h"

#include "core/ByteReader.h"

#include <algorithm>

namespace formats {

    namespace {

        constexpr std::size_t kRecordSize = 148; // bytes per NPC.Dat record
        constexpr std::size_t kNameLength = 20; // bytes of the name field (after its length byte)

        /// @brief Decodes a stored byte: (v xor 200) - 1.
        /// @param value Stored value.
        /// @return The real value.
        std::uint8_t decodeByte(std::uint8_t value) { return static_cast<std::uint8_t>((value ^ 200) - 1); } // 200: byte key

        /// @brief Decodes a stored word: (v xor 21001) - 1.
        /// @param value Stored value.
        /// @return The real value.
        std::uint16_t decodeWord(std::uint16_t value) { return static_cast<std::uint16_t>((value ^ 21001) - 1); } // 21001: word key

        /// @brief Decodes a stored integer: (v xor 195999510) - 1.
        /// @param value Stored value.
        /// @return The real value.
        std::int32_t decodeInt(std::uint32_t value) { return static_cast<std::int32_t>((value ^ 195999510u) - 1); } // 195999510: integer key

    }

    std::optional<NpcTable> NpcTable::parse(const core::Bytes& data) {
        if (data.size() < 2 * kRecordSize) { // header record plus at least one NPC record
            return std::nullopt;
        }
        NpcTable table;
        for (std::size_t at = kRecordSize; at + kRecordSize <= data.size(); at += kRecordSize) {
            core::ByteReader reader(data, at);
            NpcInfo npc;
            const std::size_t length = std::min<std::size_t>(reader.u8(), kNameLength);
            std::string name = reader.text(kNameLength);
            std::reverse(name.begin(), name.end());
            npc.name = name.substr(0, length);
            npc.type = decodeByte(reader.u8());
            npc.npcId = decodeWord(reader.u16());
            npc.spriteId = decodeWord(reader.u16());
            npc.roleSpriteId = decodeWord(reader.u16());
            for (std::int32_t& color : npc.colors) {
                color = decodeInt(reader.u32());
            }
            reader.seek(at + 44); // record offset of the uncatchable flag
            npc.uncatchable = decodeByte(reader.u8());
            npc.bodyType = decodeByte(reader.u8());
            npc.weaponType = decodeByte(reader.u8());
            constexpr int kFirstCatchable = 17767; // NPC 17767: Butterfly (first of four Butterfly NPCs forced catchable)
            constexpr int kLastCatchable = 17770; // NPC 17770: Butterfly (last of the four)
            reader.seek(at + 47); // record offset of the level
            npc.level = decodeByte(reader.u8());
            npc.barHp = decodeInt(reader.u32());
            npc.collectionNumber = decodeInt(reader.u32());
            npc.strength = decodeWord(reader.u16());
            npc.constitution = decodeWord(reader.u16());
            npc.intelligence = decodeWord(reader.u16());
            npc.wisdom = decodeWord(reader.u16());
            npc.agility = decodeWord(reader.u16());
            reader.seek(at + 66); // record offset of the height mode
            npc.heightMode = decodeByte(reader.u8());
            npc.element = decodeByte(reader.u8());
            for (int& skill : npc.skills) {
                skill = decodeWord(reader.u16());
            }
            reader.seek(at + 84); // record offset of the shadow
            npc.shadow = decodeByte(reader.u8());
            reader.seek(at + 93); // record offset of the direction kind
            npc.directionKind = decodeByte(reader.u8());
            reader.seek(at + 96); // record offset of the weapon flag
            npc.weapon = decodeByte(reader.u8());
            npc.spriteSize = decodeByte(reader.u8());
            npc.faceSpriteId = decodeWord(reader.u16());
            npc.npcKey = decodeWord(reader.u16());
            npc.stateSoundNo = decodeWord(reader.u16());
            reader.seek(at + 106); // record offset of the fixed skill
            npc.fixedSkill = decodeWord(reader.u16());
            reader.seek(at + 109); // record offset of the grow key
            npc.growKey = decodeByte(reader.u8());
            npc.noPk = decodeByte(reader.u8());
            reader.seek(at + 117); // record offset of the multi-HP-bar flag
            npc.multiHpBars = decodeByte(reader.u8());
            reader.seek(at + 120); // record offset of the mercenary flag
            npc.mercenary = decodeWord(reader.u16());
            if (npc.npcId >= kFirstCatchable && npc.npcId <= kLastCatchable) {
                npc.uncatchable = 0;
            }
            if (npc.npcId != 0) {
                if (table.m_npcs.emplace(npc.npcId, npc).second) {
                    if (npc.npcKey != 0) {
                        table.m_keys.emplace(npc.npcKey, npc.npcId);
                    }
                    if (npc.growKey != 0) {
                        table.m_growKeys.emplace(npc.growKey, npc.npcId);
                    }
                }
            }
        }
        return table;
    }

    std::optional<NpcInfo> NpcTable::find(int npcId) const {
        const auto found = m_npcs.find(npcId);
        if (found == m_npcs.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    std::optional<NpcInfo> NpcTable::findByKey(int npcKey) const {
        const auto found = m_keys.find(npcKey);
        return found == m_keys.end() ? std::nullopt : find(found->second);
    }

    std::optional<NpcInfo> NpcTable::findByGrowKey(int growKey) const {
        const auto found = m_growKeys.find(growKey);
        return found == m_growKeys.end() ? std::nullopt : find(found->second);
    }

    bool NpcTable::setName(int npcId, const std::string& name) {
        const auto found = m_npcs.find(npcId);
        if (found == m_npcs.end()) {
            return false;
        }
        found->second.name = name;
        return true;
    }

}
