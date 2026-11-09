#include "client/data/CareerTexts.h"

namespace client {

    std::string careerName(int career) {
        switch (career) {
            case 1: return "Killer "; // career 1
            case 2: return "Warrior "; // career 2
            case 3: return "Knight "; // career 3
            case 4: return "Wit "; // career 4
            case 5: return "Priest "; // career 5
            case 6: return "Seer "; // career 6
            default: return {}; // unknown career: no name
        }
    }

    std::string careerDescription(int career) {
        switch (career) {
            case 1: // career 1: Killer description
                return "    The best defence is to attack. The players experiencedclasses choosing and became killers. Killing the army is the most important quest for them. On the battlefieldKilling the leader of the army is crucial for the war.";
            // career 2: Warrior description
            case 2: return "    Equipped with armorplayers can make a effective defenseAll warriors will be strongerAnd the army will be scared";
            case 3: // career 3: Knight description
                return "    Speed is a key feature of the knight\xA1"
                       "AThey are also good at ridingOn the battlefield they own reputationand also they are the indicator in the war";
            case 4: // career 4: Wit description
                return "   Mages are good at spell attacks. And they'll be Wits after choosing classAnd also wits have stronger spellThey are resourcefuland can deal with different situations. ThereforeMages play a significant role on a team";
            case 5: // career 5: Priest description
                return "    They are a guild having strong faith and beliefs. Mages,they are wise and alsothey can resist different attacks effectively.Therefore they can provide a team with powful defense andendurance";
            // career 6: Seer description
            case 6: return "    Seers can know what will happen in future They can use a paternoster and defeat armies unexpectedly";
            default: return {}; // unknown career: no description
        }
    }

    std::string careerBonusText(int career) {
        const std::string bullet = "\xA3\xBB";
        switch (career) {
            // career 1: Killer bonuses
            case 1:
                return bullet + "ATK points(except equipment) will increase 10% " + bullet + "Target's counterattack rate decreased " + bullet +
                       "Primary spell increased ";
            // career 2: Warrior bonuses
            case 2: return bullet + "DEF points (except equipment) will increase 10% " + bullet + "Defensive ability increased ";
            // career 3: Knight bonuses
            case 3: return bullet + "SPD points (except equipment and saddle) will increase 10% " + bullet + "Increasing riding speed without saddle ";
            // career 4: Wit bonuses
            case 4: return bullet + "MATK points(except equipment) will increase 10% " + bullet + "Sealed skill rate increased ";
            case 5: // career 5: Priest bonuses
                return bullet + "MDEF points(except equipment) will increase 10% " + bullet + "Strong MDEF sealed resistent skill " + bullet +
                       "Strong attack and defense of MDEF ";
            // career 6: Seer bonuses
            case 6: return bullet + "SPD points(except equipment) will increase 10% " + bullet + "Unsuccessful rate spell and sealed skill  decreased ";
            default: return {}; // unknown career: no bonus text
        }
    }

    int careerChosenPose(int career) {
        switch (career) {
            case 1: // killer
            case 2: return 34; // warrior; killer and warrior face pose 34
            case 3: return 32; // knight faces pose 32
            case 4: // wit
            case 5: // priest
            case 6: return 42; // seer; wit, priest and seer face pose 42
            default: return kCareerRestPose; // unknown career: the rest pose
        }
    }

}
