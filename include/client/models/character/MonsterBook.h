#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_MONSTER_BOOK_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_MONSTER_BOOK_H

#include "core/Bytes.h"
#include "formats/NpcInfo.h"
#include "formats/NpcTable.h"

#include <bitset>
#include <vector>

namespace client {

    /// @brief The monsters the player has met: a bit per monster book key, listed in key order, ten to a page.
    /// @details Original: Player.FCollectionBits, TTH_CellectionForm.FEntries.
    class MonsterBook {
        public:
            static constexpr int kMostKeys = 4500; // most collection keys (monster entries) in the book
            static constexpr int kPerPage = 10; // entries shown on one page of the collection window

            /// @brief Creates an empty book over the NPC table.
            /// @param npcs NPC table that maps keys to monsters.
            explicit MonsterBook(
                const formats::NpcTable& npcs
            )
                : m_npcs(npcs) {}

            /// @brief Marks a monster as met.
            /// @details Original: HandleCollectionAddPacket (server 53/9).
            /// @param npcId NPC id of the monster.
            void add(int npcId);

            /// @brief Adds the met monsters of a bit packet: pairs of a group byte and its eight keys' bits.
            /// @details Original: HandleCollectionBitsPacket (server 53/10).
            /// @param payload The packet payload.
            void addBits(const core::Bytes& payload);

            /// @brief Number of monsters in the book.
            /// @return Entry count.
            int entryCount() const { return static_cast<int>(m_entries.size()); }

            /// @brief Number of pages.
            /// @details Original: FPageCount.
            /// @return Page count.
            int pageCount() const { return entryCount() / kPerPage + 1; }

            /// @brief A book entry; entries are numbered from 1.
            /// @param number Entry number from 1.
            /// @return The monster's NPC info.
            const formats::NpcInfo& entry(int number) const { return m_entries[static_cast<std::size_t>(number - 1)]; }

        private:
            /// @brief Rebuilds the entry list from the known bits.
            void rebuild();

            const formats::NpcTable& m_npcs;
            std::bitset<kMostKeys + 1> m_known;
            std::vector<formats::NpcInfo> m_entries;
    };

}

#endif
