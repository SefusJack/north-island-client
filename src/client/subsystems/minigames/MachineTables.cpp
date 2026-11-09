#include "client/subsystems/minigames/MachineTables.h"

namespace client {

    namespace {

        // Fortune Egg prize item ids; index 0 = no prize
        constexpr MachineTables::Prizes kEgg{
            0,
            32162, // prize 1: Pizza
            46004, // prize 2: Phoenix's Feather
            33044, // prize 3: Hot Sweet Potato
            34122, // prize 4: Lethe Scroll
            25211, // prize 5: SPD Ring
            22156, // prize 6: Gold Swan Helmet
            30065, // prize 7: Turtle Car Voucher
            30066, // prize 8: Motorbike Ticket
            33028, // prize 9: Mighty Grenade
            22166 // prize 10: Snow Ears
        };
        // slot machine kind 8 prize item ids; index 0 = the no-prize consolation item
        constexpr MachineTables::Prizes kSlot8{
            30063, // no prize: Lottery Ticket
            61044, // prize 1: Light Diamond
            61039, // prize 2: Death Diamond
            61037, // prize 3: Blove Diamond
            61035, // prize 4: Grass Diamond
            61033, // prize 5: Golden Diamond
            61032, // prize 6: Pink Diamond
            61031, // prize 7: Nimbus Diamond
            61030, // prize 8: Shining Diamond
            61028, // prize 9: Violet Diamond
            61026 // prize 10: Red Diamond
        };
        // slot machine kind 10 prize item ids; index 0 = the no-prize consolation item
        constexpr MachineTables::Prizes kSlot10{
            32095, // no prize: Fugu Hot Pot
            34123, // prize 1: 8X EXP Potion
            34116, // prize 2: 6X EXP Potion
            34115, // prize 3: 5.5X EXP Potion
            34114, // prize 4: 5X EXP Potion
            34112, // prize 5: 4X EXP Potion
            34111, // prize 6: 3.5X EXP Potion
            34110, // prize 7: 3X EXP Potion
            34109, // prize 8: 2.5X EXP Potion
            34108, // prize 9: 2X EXP Potion
            34107 // prize 10: 1.5X EXP Potion
        };
        // slot machine kind 19 prize item ids; index 0 = the no-prize consolation item
        constexpr MachineTables::Prizes kSlot19{
            30063, // no prize: Lottery Ticket
            62011, // prize 1: 32% Bomb Diamond
            62010, // prize 2: 28% Bomb Diamond
            62007, // prize 3: 22% Bomb Diamond
            62006, // prize 4: 20% Bomb Diamond
            62005, // prize 5: 18% Bomb Diamond
            62004, // prize 6: 16% Bomb Diamond
            62003, // prize 7: 14% Bomb Diamond
            62002, // prize 8: 12% Bomb Diamond
            62001, // prize 9: 10% Bomb Diamond
            62000 // prize 10: 8% Bomb Diamond
        };
        // y of each prize line in a slot machine column, px (index 0 unused)
        constexpr std::array<int, 6> kSlotLines{0, 112, 170, 222, 277, 332};

    }

    const MachineTables::Prizes& MachineTables::eggPrizes() { return kEgg; }

    const MachineTables::Prizes& MachineTables::slotPrizes(int kind) {
        switch (kind) {
            case 10: return kSlot10; // slot machine kind 10
            case 19: return kSlot19; // slot machine kind 19
            default: return kSlot8; // slot machine kind 8
        }
    }

    core::Point MachineTables::eggPrizeAt(int prize) {
        constexpr std::array<int, 6> kRows{0, 280, 230, 180, 130, 80}; // y of each egg prize box in a column, px (50 px apart, index 0 unused)
        const int row = prize <= 5 ? prize : prize - 5; // prizes 1-5 are one column, 6-10 the other
        return {prize <= 5 ? 440 : 160, kRows[static_cast<std::size_t>(row)]}; // x of the right (1-5) and left (6-10) column
    }

    core::Point MachineTables::slotPrizeAt(int prize) {
        const int row = prize <= 5 ? prize : prize - 5; // prizes 1-5 are one column, 6-10 the other
        return {prize <= 5 ? 278 : 491, kSlotLines[static_cast<std::size_t>(row)]}; // x of the left (1-5) and right (6-10) column, px
    }

    core::Point MachineTables::reelAt(int reel) {
        constexpr std::array<int, 4> kLeft{0, 129, 306, 485}; // x of reels 1-3, px (index 0 unused)
        return {kLeft[static_cast<std::size_t>(reel)], 410}; // y of the reels
    }

    std::uint32_t MachineTables::reelStopMs(int reel) {
        constexpr std::array<std::uint32_t, 4> kStops{0, 1800, 3000, 4200}; // ms after the spin starts that reels 1-3 stop (index 0 unused)
        return kStops[static_cast<std::size_t>(reel)];
    }

}
