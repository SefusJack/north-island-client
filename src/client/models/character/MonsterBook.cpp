#include "client/models/character/MonsterBook.h"

#include "core/ByteReader.h"

namespace client {

    void MonsterBook::add(int npcId) {
        if (const auto npc = m_npcs.find(npcId); npc && npc->npcKey > 0 && npc->npcKey <= kMostKeys) {
            m_known.set(static_cast<std::size_t>(npc->npcKey));
        }
        rebuild();
    }

    void MonsterBook::addBits(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip the 2-byte packet header (action, sub id)
        while (reader.remaining() >= 2) {
            const int group = reader.u8();
            const int bits = reader.u8();
            // each group byte covers eight keys, one bit each
            for (int i = 1; i <= 8; ++i) {
                const int key = (group - 1) * 8 + i;
                if ((bits >> (i - 1) & 1) != 0 && key >= 1 && key <= kMostKeys) {
                    m_known.set(static_cast<std::size_t>(key));
                }
            }
        }
        rebuild();
    }

    void MonsterBook::rebuild() {
        m_entries.clear();
        for (int key = 1; key <= kMostKeys; ++key) {
            if (m_known.test(static_cast<std::size_t>(key))) {
                if (const auto npc = m_npcs.findByKey(key)) {
                    m_entries.push_back(*npc);
                }
            }
        }
    }

}
