#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_DEFENSE_NPC_PREFIXES_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_DEFENSE_NPC_PREFIXES_H

#include <array>
#include <string>
#include <utility>
#include <vector>

namespace client {

    /// @brief The words put before the names of the castle defense NPCs of the three Trojan War castles, four slots a castle.
    /// @details Original: CastleManage.FCastleData.DefenseNpcPrefixes, filled by RecvDefenseNpcPrefix*.
    class DefenseNpcPrefixes {
        public:
            using Entry = std::pair<int, std::string>;

            /// @brief Clears the castle's prefixes, then puts the entries.
            /// @details Original: RecvDefenseNpcPrefixList.
            /// @param castle Castle number (1..3).
            /// @param entries Slot and prefix pairs.
            void list(int castle, const std::vector<Entry>& entries);

            /// @brief Puts the entries into the castle's slots.
            /// @details Original: RecvDefenseNpcPrefixUpdate.
            /// @param castle Castle number (1..3).
            /// @param entries Slot and prefix pairs.
            void update(int castle, const std::vector<Entry>& entries);

            /// @brief Clears one slot of a castle.
            /// @details Original: RecvDefenseNpcPrefixRemoved.
            /// @param castle Castle number (1..3).
            /// @param slot Slot (1..4).
            void remove(int castle, int slot);

            /// @brief The prefix of the n-th defense NPC drawn on a Trojan War map (the slots that have a prefix are counted in turn).
            /// @details Original: GetDefenseNpcPrefix.
            /// @param mapId Map id of a Trojan War map.
            /// @param ordinal Position of the NPC (from 1).
            /// @return The prefix, empty for none.
            std::string prefix(int mapId, int ordinal) const;

        private:
            static constexpr int kSlots = 4;

            /// @brief Whether a castle number is valid.
            /// @param castle Castle number.
            /// @return True for 1..3.
            static bool validCastle(int castle) { return castle >= 1 && castle <= 3; } // three Trojan War castles

            std::array<std::array<std::string, kSlots>, 3> m_prefixes; // three castles
    };

}

#endif
