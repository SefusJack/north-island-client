#include "client/data/JobGrantSkills.h"

#include <array>
#include <cstdint>

namespace client {

    namespace {

        constexpr int kElements = 5; // elements 0..4 (0 none, 1 Earth, 2 Water, 3 Fire, 4 Wind)
        constexpr int kGroups = 4; // groups per element (the client uses groups 1..3)
        constexpr int kRows = 11; // rows per group (the client uses rows 1..10)
        constexpr int kColumns = 6; // columns per row (column 1 the skill, columns 2..5 what it grants)

        // element 0 and group 0 are unused (all zero); each line below holds 4 rows, and its comment reads
        // element group row: skill -> the skills it grants (a row with no comment is empty)
        // skill ids, 6 columns per row (column 1 the skill, the next what it grants), 11 rows per group, 4 groups per element
        constexpr std::array<std::uint16_t, kElements * kGroups * kRows * kColumns> kTable = {
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     15085, 11085, 15074, 0,     0,     0, 11017, 11086, 11065, 0,     0,     0, 11087, 11071, 11088,
            0,     0,     0,     15083, 12025, 15084, 0,     0, // Earth g1 r1: Rock Attack -> Rock Hit, Rock Beating | Earth g1 r2: Earth Attack -> Earth Hit, Earth Beating | Earth g1 r3: Rupture Attack -> Rupture Hit, Rupture Beating | Earth g1 r4: Boulder Attack -> Boulder Hit, Boulder Beating
            0,     15049, 15020, 15043, 0,     0,     0,     15146, 15144, 15147, 0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Earth g1 r5: Jump Attack -> Jump Hit, Jump Beating | Earth g1 r6: Scurry Attack -> Scurry Hit, Scurry Beating
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 12006, 21301, 15094,
            0,     0, // Earth g2 r1: Rock Elf Attack -> Rock Elf Hit, Rock Elf Beating
            0,     15056, 11090, 11091, 0,     0,     0,     11019, 11093, 11092, 0,     0,     0, 11031, 11094, 11105, 0,     0,     0, 15086, 15087, 15088,
            0,     0, // Earth g2 r2: Rockfall Attack -> Rockfall Hit, Rockfall Beating | Earth g2 r3: Rock Blast Attack -> Rock Blast Hit, Rock Blast Beating | Earth g2 r4: Rock Ball Attack -> Rock Ball Hit, Rock Ball Beating | Earth g2 r5: Stress Attack -> Stress Hit, Stress Beating
            0,     11107, 15152, 15153, 0,     0,     0,     12018, 11061, 15154, 0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Earth g2 r6: Megalith Attack -> Megalith Hit, Megalith Beating | Earth g2 r7: Acrolith Attack -> Aerolite Hit, Aerolite Beating
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 11057, 0,     0,     0,     0,     0, 12043, 0,     0,
            0,     0, // Earth g3 r1: Shield Defence | Earth g3 r2: Wake Spell
            0,     12048, 0,     0,     0,     0,     0,     11055, 0,     0,     0,     0,     0, 15070, 0,     0,     0,     0,     0, 15035, 0,     0,
            0,     0, // Earth g3 r3: Ice-out | Earth g3 r4: Stone Wall | Earth g3 r5: Tree Bind | Earth g3 r6: Terrafication
            0,     15066, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Earth g3 r7: Summon Mosquito
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     15091, 15092, 15093, 0,     0,     0, 11001, 15062, 15063, 0,     0,     0, 11044, 11004, 11096,
            0,     0,     0,     15019, 15059, 15073, 0,     0, // Water g1 r1: Ice Attack -> Ice Hit, Ice Beating | Water g1 r2: Icicle Attack -> Icicle Hit, Icicle Beating | Water g1 r3: Water Attack -> Water Hit, Water Beating | Water g1 r4: Turning Ice Attack -> Turning Ice Hit, Turning Ice Beating
            0,     12007, 11010, 21201, 0,     0,     0,     15156, 15155, 15157, 0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Water g1 r5: Frost Attack -> Frost Hit, Frost Beating | Water g1 r6: Eddy Attack -> Eddy Hit, Eddy Beating
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 15097, 15096, 15095,
            0,     0, // Water g2 r1: Water Pole Attack -> Water Pole Hit, Water Pole Beating
            0,     11040, 11022, 11100, 0,     0,     0,     11110, 15098, 15099, 0,     0,     0, 11113, 12002, 11058, 0,     0,     0, 11024, 15010, 15011,
            0,     0, // Water g2 r2: Chill Attack -> Chill Hit, Chill Beating | Water g2 r3: Ice Ball Attack -> Ice Ball Hit, Ice Ball Beating | Water g2 r4: Ice Elf Attack -> Ice Elf Hit, Ice Elf Beating | Water g2 r5: Thunder Attack -> Thunder Hit, Thunder Beating
            0,     15158, 15159, 15160, 0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Water g2 r6: Water Thunder Attack -> Water Thunder Hit, W.T. Beating
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 15100, 0,     0,     0,     0,     0, 11042, 0,     0,
            0,     0, // Water g3 r1: Detoxifcation | Water g3 r2: Cure Spell
            0,     15075, 0,     0,     0,     0,     0,     11080, 0,     0,     0,     0,     0, 11051, 0,     0,     0,     0,     0, 11043, 0,     0,
            0,     0, // Water g3 r3: Magic Recovery | Water g3 r4: Water Shield | Water g3 r5: Freezing Spell | Water g3 r6: Recovery
            0,     12042, 0,     0,     0,     0,     0,     12041, 0,     0,     0,     0,     0, 11053, 0,     0,     0,     0,     0, 15042, 0,     0,
            0,     0, // Water g3 r7: Free From mess | Water g3 r8: Sleep | Water g3 r9: Revival | Water g3 r10: Recover Normal
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     11016, 11029, 11025, 0,     0,     0, 15101, 20003, 15089, 0,     0,     0, 11114, 11115, 12022,
            0,     0,     0,     12039, 12034, 12038, 0,     0, // Fire g1 r1: Flame Attack -> Flame Hit, Flame Beating | Fire g1 r2: Sword Awn Attack -> Sword Awn Hit, Sword Awn Beating | Fire g1 r3: Turning Fire Attack -> Turning Fire Hit, Turning Fire Beating | Fire g1 r4: Fire Wave Attack -> Fire Wave Hit, Fire Wave Beating
            0,     15102, 15015, 15103, 0,     0,     0,     15044, 15017, 15045, 0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Fire g1 r5: Five Star Attack -> Five Star Hit, Five Star Beating | Fire g1 r6: Hagendis Attack -> Hagendis Hit, Hagendis Beating
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 11166, 15104, 15105,
            0,     0, // Fire g2 r1: Blast Attack -> Blast Hit, Blast Beating
            0,     11005, 30000, 15110, 0,     0,     0,     11034, 11117, 15106, 0,     0,     0, 15109, 15107, 15108, 0,     0,     0, 15111, 12021, 12024,
            0,     0, // Fire g2 r2: Fire Blast Attack -> Fire Blast Hit, Fire Blast Beating | Fire g2 r3: Fire Ball Attack -> Fire Ball Hit, Fire Ball Beating | Fire g2 r4: Blaze Attack -> Blaze Hit, Blaze Beating | Fire g2 r5: Fire Combo Attack -> Fire Combo Hit, Fire Combo Beating
            0,     11035, 15112, 11059, 0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Fire g2 r6: Fire Stone Attack -> Fire Stone Hit, Fire Stone Bead
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 11056, 0,     0,     0,     0,     0, 11002, 0,     0,
            0,     0, // Fire g3 r1: Slowdown | Fire g3 r2: Poison Spell
            0,     11072, 0,     0,     0,     0,     0,     12045, 0,     0,     0,     0,     0, 11003, 0,     0,     0,     0,     0, 15171, 0,     0,
            0,     0, // Fire g3 r3: Fiery Attack | Fire g3 r4: Free From Coma | Fire g3 r5: Mess Spell | Fire g3 r6: Fire Attack
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,     0,     0,     0,     0,     0,     0,     0,     11007, 11014, 15113, 0, 0,     0,     15079, 15080, 15081, 0, 0,     0,     15117,
            12009, 15129, 0,     0,     0,     15002, 11045, 15061, 0,     0, // Wind g1 r1: Wind Attack -> Wind Hit, Wind Bead | Wind g1 r2: Air Attack -> Air Hit , Air Bead | Wind g1 r3: Cyclone Attack -> Cyclone Hit, Cyclone Beating | Wind g1 r4: Instant Attack -> Instant Hit, Instant Beating
            0,     11046, 11098, 12032, 0,     0,     0,     15114, 15115, 15116, 0,     0,     0, 15128, 15130, 15131, 0,     0,     0, 15120, 15121, 11012,
            0,     0, // Wind g1 r5: Wind Stress Attack -> Wind Stress Hit, Wind Stress Beating | Wind g1 r6: Dead Wind Attack -> Dead Wind Hit, Dead Wind Beating | Wind g1 r7: Shadow Attack -> Shadow Hit, Shadow Beating | Wind g1 r8: Afterimage Attack -> Afterimage Hit, Afterimage Beating
            0,     21401, 11023, 20001, 0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 30002, 15122, 11050,
            0,     0, // Wind g1 r9: Storm Attack -> Storm Hit, Storm Beating | Wind g2 r1: Wind Blade Attack -> Wind Blade Hit, Wind Blade Beating
            0,     11015, 15009, 15018, 0,     0,     0,     15123, 15000, 15124, 0,     0,     0, 15125, 15126, 15127, 0,     0,     0, 15048, 15050, 15051,
            0,     0, // Wind g2 r2: Wind Cut Attack -> Wind Cut Hit, Wind Cut Beating | Wind g2 r3: Whirlwind Attack -> Whirlwind Hit, Whirlwind Beating | Wind g2 r4: Space Attack -> Space Hit, Space Beating | Wind g2 r5: Sky Attack -> Sky Hit, Sky Beating
            0,     15161, 15162, 15163, 0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0, // Wind g2 r6: Air Stress Attack -> Air Stress Hit, Aire Stress Beating
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 11052, 0,     0,     0,     0,     0, 11073, 0,     0,
            0,     0, // Wind g3 r1: Speedup | Wind g3 r2: Shield Smash
            0,     12046, 0,     0,     0,     0,     0,     15032, 0,     0,     0,     0,     0, 15036, 0,     0,     0,     0,     0, 11026, 0,     0,
            0,     0, // Wind g3 r3: Unload Wall | Wind g3 r4: Cord Spell | Wind g3 r5: Unstun | Wind g3 r6: Coma Spell
            0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0,     0,     0,     0,     0,     0, 0,     0,     0,
            0,     0,
        };

    }

    int jobGrantSkill(int element, int group, int row, int column) {
        if (element < 0 || element >= kElements || group < 0 || group >= kGroups || row < 0 || row >= kRows || column < 0 || column >= kColumns) {
            return 0;
        }
        return kTable[static_cast<std::size_t>(((element * kGroups + group) * kRows + row) * kColumns + column)];
    }

}
