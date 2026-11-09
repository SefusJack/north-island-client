#ifndef NORTH_ISLAND_CLIENT_MODELS_REMOTE_AUTO_BATTLE_SETTINGS_H
#define NORTH_ISLAND_CLIENT_MODELS_REMOTE_AUTO_BATTLE_SETTINGS_H

#include <array>
#include <cstdint>

namespace client {

    /// @brief What the battle's auto button does for the player (set 1) or his pet (set 2) with the Remote Control Plus.
    /// @details Original: TAutoBattleSettings, set in HL_Menu.pas THL_AutoPlayFormPlus, run by RunAutoBattleSettings; arrays are numbered from 1 as the game's, [0] unused.
    struct AutoBattleSettings {
        std::array<std::uint8_t, 7> flags{};
        std::array<int, 3> defaultSkillIds{};
        std::array<int, 4> procSkillIds{};
        std::array<int, 4> procSkillIsItem{};
        std::array<int, 4> procSkillTargets{};
        std::array<int, 4> healThresholds{};
        std::array<int, 4> healIsItem{};
        std::array<int, 4> healActionIds{};
        std::array<int, 4> healTargets{};
        int faintCompareMode = 0;
        int faintHpValue = 0;
        int faintActionId = 0;
        int faintTarget = 0;

        /// @brief Gives the settings the form starts with (original THL_AutoPlayFormPlus.ResetAutoPlaySettings).
        /// @return Settings with the default targets.
        static AutoBattleSettings defaults() {
            AutoBattleSettings settings;
            settings.procSkillTargets = {0, 1, 1, 1}; // default target code of each named skill (index 0 unused)
            settings.faintTarget = 1;
            settings.healTargets[1] = 4; // default target code of the HP heal
            settings.healTargets[2] = 4; // default target code of the SP heal
            settings.faintCompareMode = 1;
            return settings;
        }

        /// @brief Compares the fields the form compares (original CheckAutoPlayModified; not the items' flags or the compare mode).
        /// @param other Settings to compare with.
        /// @return True when the compared fields are equal.
        bool sameForForm(const AutoBattleSettings& other) const {
            return flags == other.flags && defaultSkillIds == other.defaultSkillIds && procSkillIds == other.procSkillIds &&
                   procSkillTargets == other.procSkillTargets && healThresholds == other.healThresholds && healIsItem == other.healIsItem &&
                   healActionIds == other.healActionIds && healTargets == other.healTargets && faintHpValue == other.faintHpValue &&
                   faintActionId == other.faintActionId && faintTarget == other.faintTarget;
        }
    };

}

#endif
