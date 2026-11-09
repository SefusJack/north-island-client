#ifndef NORTH_ISLAND_CLIENT_RULES_PET_INFO_LINES_H
#define NORTH_ISLAND_CLIENT_RULES_PET_INFO_LINES_H

#include "formats/NpcInfo.h"
#include "protocol/PetInfoBlock.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Builds the lines of the pet info popup.
    /// @details Original: TCY_PopFNpcInfoForm.ShowPetInfo.
    /// @param pet The pet's info block.
    /// @param maxHp The pet's maximal HP.
    /// @param maxSp The pet's maximal SP.
    /// @param npc The pet's NPC record, if known.
    /// @param skillName Gives a skill's name from its id.
    /// @return The popup lines.
    std::vector<std::string> petInfoLines(
        const protocol::PetInfoBlock& pet,
        int maxHp,
        int maxSp,
        const std::optional<formats::NpcInfo>& npc,
        const std::function<std::string(int)>& skillName
    );

}

#endif
