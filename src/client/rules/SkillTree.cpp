#include "client/rules/SkillTree.h"

namespace client {

    namespace {

        const std::array<std::array<SkillTreeCell, kSkillTreeCells>, 4> kTrees = {{
            // per element 1..4 (earth, water, fire, wind): 18 cells, three columns of six; {skill id, {STR, CON, INT, WIS, AGI needed}}
            // element 1: earth
            {{{15085, {0, 0, 0, 0, 0}}, // Rock Attack
              {11017, {16, 0, 0, 0, 0}}, // Earth Attack
              {11087, {28, 0, 0, 0, 0}}, // Rupture Attack
              {15083, {44, 0, 0, 2, 0}}, // Boulder Attack
              {15049, {60, 5, 0, 0, 0}}, // Jump Attack
              {15146, {80, 0, 0, 0, 4}}, // Scurry Attack
              {12006, {0, 0, 0, 0, 0}}, // Rock Elf Attack
              {15056, {0, 0, 16, 0, 0}}, // Rockfall Attack
              {11019, {0, 0, 28, 0, 0}}, // Rock Blast Attack
              {11031, {0, 0, 46, 0, 0}}, // Rock Ball Attack
              {15086, {0, 0, 67, 0, 0}}, // Stress Attack
              {11107, {0, 0, 90, 3, 0}}, // Megalith Attack
              {11057, {0, 0, 0, 0, 0}}, // Shield Defence
              {12043, {0, 0, 0, 11, 0}}, // Wake Spell
              {12048, {0, 0, 0, 18, 0}}, // Ice-out
              {11055, {0, 0, 0, 29, 0}}, // Stone Wall
              {15070, {0, 4, 0, 38, 0}}, // Tree Bind
              {15035, {0, 0, 4, 48, 0}}}}, // Terrafication
            // element 2: water
            {{{15091, {0, 0, 0, 0, 0}}, // Ice Attack
              {11001, {13, 0, 0, 0, 0}}, // Icicle Attack
              {11044, {19, 0, 1, 0, 0}}, // Water Attack
              {15019, {27, 0, 2, 0, 0}}, // Turning Ice Attack
              {12007, {35, 0, 3, 0, 0}}, // Frost Attack
              {15156, {58, 0, 4, 0, 0}}, // Eddy Attack
              {15097, {0, 0, 0, 0, 0}}, // Water Pole Attack
              {11040, {0, 0, 16, 0, 0}}, // Chill Attack
              {11110, {0, 0, 28, 0, 0}}, // Ice Ball Attack
              {11113, {0, 0, 46, 0, 0}}, // Ice Elf Attack
              {11024, {0, 0, 65, 2, 0}}, // Thunder Attack
              {15158, {0, 0, 85, 6, 0}}, // Water Thunder Attack
              {15100, {0, 0, 0, 0, 0}}, // Detoxifcation
              {11042, {0, 0, 0, 13, 0}}, // Cure Spell
              {15075, {0, 0, 0, 19, 0}}, // Magic Recovery
              {11080, {0, 0, 0, 25, 0}}, // Water Shield
              {11051, {0, 0, 1, 30, 0}}, // Freezing Spell
              {11043, {0, 0, 0, 36, 0}}}}, // Recovery
            // element 3: fire
            {{{11016, {0, 0, 0, 0, 0}}, // Flame Attack
              {15101, {16, 0, 0, 0, 0}}, // Sword Awn Attack
              {11114, {28, 0, 0, 0, 0}}, // Turning Fire Attack
              {12039, {46, 0, 0, 0, 0}}, // Fire Wave Attack
              {15102, {65, 0, 0, 2, 0}}, // Five Star Attack
              {15044, {85, 0, 0, 0, 4}}, // Hagendis Attack
              {11166, {0, 0, 0, 0, 0}}, // Blast Attack
              {11005, {0, 0, 15, 2, 0}}, // Fire Blast Attack
              {11034, {0, 0, 27, 0, 0}}, // Fire Ball Attack
              {15109, {0, 0, 45, 0, 0}}, // Blaze Attack
              {15111, {0, 0, 66, 0, 0}}, // Fire Combo Attack
              {11035, {0, 0, 86, 6, 0}}, // Fire Stone Attack
              {11056, {0, 0, 0, 0, 0}}, // Slowdown
              {11002, {0, 0, 0, 11, 0}}, // Poison Spell
              {11072, {0, 0, 0, 18, 0}}, // Fiery Attack
              {12045, {0, 0, 0, 29, 0}}, // Free From Coma
              {11003, {0, 0, 0, 42, 0}}, // Mess Spell
              {15171, {0, 0, 1, 55, 0}}}}, // Fire Attack
            // element 4: wind
            {{{11007, {0, 0, 0, 0, 0}}, // Wind Attack
              {15079, {5, 0, 0, 0, 10}}, // Air Attack
              {15117, {10, 0, 0, 0, 17}}, // Cyclone Attack
              {15002, {0, 0, 0, 0, 35}}, // Instant Attack
              {11046, {20, 0, 0, 0, 46}}, // Wind Stress Attack
              {15114, {35, 0, 0, 0, 55}}, // Dead Wind Attack
              {30002, {0, 0, 0, 0, 0}}, // Wind Blade Attack
              {11015, {0, 0, 15, 0, 0}}, // Wind Cut Attack
              {15123, {0, 0, 23, 5, 0}}, // Whirlwind Attack
              {15125, {0, 0, 41, 0, 0}}, // Space Attack
              {15048, {0, 0, 62, 0, 0}}, // Sky Attack
              {15161, {0, 0, 86, 0, 0}}, // Air Stress Attack
              {11052, {0, 0, 0, 0, 0}}, // Speedup
              {11073, {0, 0, 0, 11, 0}}, // Shield Smash
              {12046, {0, 0, 0, 18, 0}}, // Unload Wall
              {15032, {0, 0, 4, 25, 0}}, // Cord Spell
              {15036, {0, 0, 0, 38, 0}}, // Unstun
              {11026, {0, 0, 7, 49, 0}}}}, // Coma Spell
        }};

        constexpr std::array<int, 3> kColumns = {69, 128, 186}; // x of the three columns of icons in the skill window, px
        constexpr int kFirstRow = 129; // y of the first icon row, px
        constexpr int kRowStep = 34; // vertical distance between icon rows, px
        constexpr int kRows = 6; // icons per column

    }

    const SkillTreeCell& skillTreeCell(int element, int cell) { return kTrees[static_cast<std::size_t>(element - 1)][static_cast<std::size_t>(cell - 1)]; }

    core::Point skillTreeCellPosition(int cell) {
        const int index = cell - 1;
        return {kColumns[static_cast<std::size_t>(index / kRows)], kFirstRow + (index % kRows) * kRowStep};
    }

}
