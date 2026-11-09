#ifndef NORTH_ISLAND_CLIENT_DATA_STARTER_ROLES_H
#define NORTH_ISLAND_CLIENT_DATA_STARTER_ROLES_H

#include "client/data/StarterRole.h"

#include <array>
#include <cstdint>

namespace client {

    constexpr int kStarterRoleCount = 14; // starter characters offered at character creation (roles 1..14)

    /// @brief The wizard's role by its Btn_RolePic number.
    /// @details Original: TRE_CreateCharacter.SelectCreateCharRole.
    /// @param number Role number 1..14.
    /// @return The role.
    const StarterRole& starterRole(int number);

    /// @brief The game's role index of a body type and head (numbers roles differently from the wizard's pictures).
    /// @details Original: GetRoleIndex.
    /// @param bodyType Body type.
    /// @param head Head number.
    /// @return Role index 1..15, 0 for none.
    int roleIndex(int bodyType, int head);

    /// @brief The items a role starts with.
    /// @details Original: RoleStarterEquips.
    /// @param roleIndex Game role index.
    /// @return Six item ids, 0 = none.
    std::array<std::uint16_t, 6> starterEquipment(int roleIndex); // six equipment slots

    /// @brief Whether a player of that body type and head wears the clothes the role starts with.
    /// @details Original: the test of ShowShortMsg's FWideLayout, RoleStarterEquips[GetRoleIndex(body, head), 2] = the item worn in slot 2.
    /// @param bodyType Body type.
    /// @param head Head number.
    /// @param clothesItemId Item id worn in the clothes slot.
    /// @return True when it is the starter clothes.
    bool wearsStarterClothes(int bodyType, int head, int clothesItemId);

    /// @brief The points a role gets on top of the five the player gives.
    /// @details Original: RoleStatBonuses.
    /// @param roleIndex Game role index.
    /// @return Bonuses in the wizard's order: STR, CON, INT, WIS, AGI.
    std::array<int, 5> roleStatBonuses(int roleIndex); // five abilities: STR, CON, INT, WIS, AGI

}

#endif
