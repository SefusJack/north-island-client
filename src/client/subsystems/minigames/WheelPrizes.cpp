#include "client/subsystems/minigames/WheelPrizes.h"

#include <array>

namespace client {

    namespace {

        /// @brief One item and its count in a prize table.
        struct Entry {
            int itemId;
            int count;
        };

        // tables of {item id, count}; the first entry of each is unused
        constexpr std::array<Entry, 6> kFirst = {{
            {0, 0}, // unused
            {34238, 1}, // Seraph Scroll
            {34165, 1}, // Win Plate
            {30296, 1}, // Holy Pegasus Voucher
            {21172, 1}, // Fur Clothes (Male)
            {21173, 1} // Fur Clothes (Female)
        }};
        constexpr std::array<Entry, 11> kSecond = {{
            {0, 0}, // unused
            {34189, 1}, // 8X Holy EXP Potion
            {34243, 1}, // Cherubim Scroll
            {34244, 1}, // Ofanim Scroll
            {34245, 1}, // Virtues Scroll
            {34247, 1}, // Small Demon Scroll
            {25571, 1}, // Swift Source
            {25570, 1}, // Curse Source
            {25569, 1}, // Purple Source
            {25568, 1}, // Defence Source
            {25567, 1} // Energy Source
        }};
        constexpr std::array<Entry, 11> kThird = {{
            {0, 0}, // unused
            {25164, 1}, // Fierce Armlet
            {25166, 1}, // Rapid Armlet
            {25168, 1}, // War Armlet
            {25170, 1}, // Bright Armlet
            {34104, 1}, // Repairing Spanner
            {25176, 1}, // Power Armlet
            {25177, 1}, // Defensive Armlet
            {25178, 1}, // Brilliant Armlet
            {25179, 1}, // Original Armlet
            {25180, 1} // Fast Armlet
        }};
        constexpr std::array<Entry, 21> kOthers = {{
            {0, 0}, // unused
            {32162, 1}, // Pizza
            {33031, 2}, // Phoenix Blood (x2)
            {34022, 1}, // Tent Palette
            {34015, 1}, // Glazed Stone Bag
            {34157, 1}, // Cherry Door Voucher
            {34158, 1}, // Beach Door Voucher
            {34175, 1}, // Seasonal Suit
            {34176, 1}, // Rabbit Suit
            {34177, 1}, // Cat Suit
            {25186, 1}, // Spirit Satin
            {25187, 1}, // Cotton Scroll
            {25188, 1}, // Red Scroll
            {25189, 1}, // Mysterious Scroll
            {25190, 1}, // Swift Scroll
            {25154, 1}, // Anger Armlet
            {25156, 1}, // Fortitude Armlet
            {25158, 1}, // Sage Armlet
            {25160, 1}, // Scale Armlet
            {25162, 1}, // Swift Armlet
            {34137, 1} // Pet Lethe Scroll
        }};

        /// @brief Picks an entry of a table, or an empty prize when the index is out of range.
        /// @tparam N Number of entries in the table.
        /// @param table Prize table.
        /// @param index Index into the table.
        /// @param tier Prize tier to report.
        /// @return The prize.
        template <std::size_t N>
        WheelPrize pick(const std::array<Entry, N>& table, int index, int tier) {
            if (index < 0 || index >= static_cast<int>(N)) {
                return {0, 0, tier};
            }
            const Entry& entry = table[static_cast<std::size_t>(index)];
            return {entry.itemId, entry.count, tier};
        }

    }

    WheelPrize wheelPrizeOf(int code) {
        if (code > 0 && code <= 30) { // codes 1..30: first prize table
            return pick(kFirst, code, 1);
        }
        if (code > 30 && code <= 60) { // codes 31..60: second prize table
            return pick(kSecond, code - 30, 2);
        }
        if (code > 60 && code <= 90) { // codes 61..90: third prize table
            return pick(kThird, code - 60, 3);
        }
        return pick(kOthers, code - 90, 0); // codes above 90: ordinary prizes
    }

}
