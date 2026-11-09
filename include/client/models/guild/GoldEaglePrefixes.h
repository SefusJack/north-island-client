#ifndef NORTH_ISLAND_CLIENT_MODELS_GUILD_GOLD_EAGLE_PREFIXES_H
#define NORTH_ISLAND_CLIENT_MODELS_GUILD_GOLD_EAGLE_PREFIXES_H

#include <array>
#include <string>
#include <utility>
#include <vector>

namespace client {

    /// @brief The word put before the name of the Gold Eagle (the guild that holds it) of each of the three Trojan War castles; only slot 1 is kept.
    /// @details Original: CastleManage.FCastleData.GoldEaglePrefix, filled by RecvGoldEaglePrefix*.
    class GoldEaglePrefixes {
        public:
            using Entry = std::pair<int, std::string>;

            /// @brief Clears the castle's prefix, then puts the entries.
            /// @details Original: RecvGoldEaglePrefixList.
            /// @param castle Castle number (1..3).
            /// @param entries Slot and prefix pairs.
            void list(int castle, const std::vector<Entry>& entries);

            /// @brief Puts the entries (only slot 1 counts).
            /// @details Original: RecvGoldEaglePrefixUpdate.
            /// @param castle Castle number (1..3).
            /// @param entries Slot and prefix pairs.
            void update(int castle, const std::vector<Entry>& entries);

            /// @brief Clears the castle's prefix when slot 1 is removed.
            /// @details Original: RecvGoldEaglePrefixRemoved.
            /// @param castle Castle number (1..3).
            /// @param slot Slot removed.
            void remove(int castle, int slot);

            /// @brief The prefix of the n-th Gold Eagle drawn on a Trojan War map (only the first has one).
            /// @details Original: GetGoldEaglePrefix.
            /// @param mapId Map id of a Trojan War map.
            /// @param ordinal Position of the Gold Eagle (from 1).
            /// @return The prefix, empty for none.
            std::string prefix(int mapId, int ordinal) const;

        private:
            /// @brief Whether a castle number is valid.
            /// @param castle Castle number.
            /// @return True for 1..3.
            static bool validCastle(int castle) { return castle >= 1 && castle <= 3; } // three Trojan War castles

            std::array<std::string, 3> m_prefixes; // three castles
    };

}

#endif
