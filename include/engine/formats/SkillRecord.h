#ifndef NORTH_ISLAND_FORMATS_SKILL_RECORD_H
#define NORTH_ISLAND_FORMATS_SKILL_RECORD_H

#include <string>

namespace formats {

    /// @brief One skill record of data\Skill.dat (the fields the client uses so far).
    struct SkillRecord {
        int id = 0;
        std::string name;
        int category = 0;
        int spCost = 0;
        int tree = 0;
        int rangeKind = 0;
        int skillType = 0;
        int learnPoints = 0;
        int prerequisite = 0;
        std::string description;
        int key = 0;
        int targetType = 0;
        int iconId = 0;
        int maxLevel = 0;
        int statusEffect = 0;
        int motionId = 0;
        int statusKind = 0;
        int tintKind = 0;
    };

}

#endif
