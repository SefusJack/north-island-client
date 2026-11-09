#include "client/rules/PetInfoLines.h"

namespace client {

    namespace {

        constexpr std::size_t kWideName = 16; // name length in characters above which a blank line pads the popup

        /// @brief Name of a pet element.
        /// @param element The element number.
        /// @return The name, empty for an unknown element.
        const char* elementName(int element) {
            switch (element) {
                case 0: return "None"; // element 0: none
                case 1: return "Earth"; // element 1: earth
                case 2: return "Water"; // element 2: water
                case 3: return "Fire"; // element 3: fire
                case 4: return "Wind"; // element 4: wind
                case 5: return "Heart"; // element 5: heart
                default: return ""; // unknown element
            }
        }

    }

    std::vector<std::string> petInfoLines(
        const protocol::PetInfoBlock& pet,
        int maxHp,
        int maxSp,
        const std::optional<formats::NpcInfo>& npc,
        const std::function<std::string(int)>& skillName
    ) {
        std::vector<std::string> lines;
        const std::string name = "[" + (npc ? npc->name : std::string()) + "]";
        lines.push_back(name);
        if (name.size() > kWideName) {
            lines.push_back(std::string(kWideName, ' '));
        }
        lines.push_back("LV: " + std::to_string(pet.level));
        lines.push_back("HP: " + std::to_string(pet.hp));
        lines.push_back("SP: " + std::to_string(pet.sp));
        lines.push_back("Maximal HP: " + std::to_string(maxHp));
        lines.push_back("Maximal SP: " + std::to_string(maxSp));
        lines.push_back("Amity" + std::to_string(pet.amity));
        lines.push_back("Strength: " + std::to_string(pet.strength));
        lines.push_back("Constitution: " + std::to_string(pet.constitution));
        lines.push_back("Intelligence: " + std::to_string(pet.intelligence));
        lines.push_back("Wisdom: " + std::to_string(pet.wisdom));
        lines.push_back("Agility: " + std::to_string(pet.agility));
        lines.push_back(std::string("Attribute: ") + (npc ? elementName(npc->element) : ""));
        if (npc) {
            std::string spells;
            for (const int skill : npc->skills) {
                if (skill != 0) {
                    spells += skillName(skill) + " ";
                }
            }
            lines.push_back("Spell: " + spells);
        }
        return lines;
    }

}
