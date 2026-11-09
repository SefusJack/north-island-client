#ifndef NORTH_ISLAND_PROTOCOL_STATUS_EFFECT_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_STATUS_EFFECT_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 5/14, 5/15 or 5/16 (no data): a click on the exp, Lucky Grass or Spa Buns icon asks for the time left.
    /// @details Original: TTimeEffectStatusForm.HandleTimeEffectClick (MZ_Menu.pas:5874).
    /// @param sub Sub id: 14, 15 or 16.
    /// @return The encoded request payload.
    core::Bytes timeEffectRequest(int sub);

    /// @brief Builds request 23/111: 1 asks for the state of the Training Island item (a click on its icon), 2 teleports to the island.
    /// @param op The operation byte, 1 or 2.
    /// @return The encoded request payload.
    core::Bytes trainingIslandRequest(int op);

    /// @brief Builds request 23/119 sub 2: asks for the minutes left of the stat buffs (a click on the Strength Ability icon).
    /// @return The encoded request payload.
    core::Bytes strengthBuffRequest();

    /// @brief Builds request 23/119 sub 3: a stat buff item is used from the bag.
    /// @details Original: GetStatBuffItemCode identifies the item; sent by TRe_ItemGridForm.SendUseStatBuffItem. Layout: u8 bagSlot.
    /// @param bagSlot Bag slot of the item.
    /// @return The encoded request payload.
    core::Bytes strengthBuffUseRequest(int bagSlot);

    /// @brief Builds request 23/114 sub 3 (on) or 4 (off): the body shift is switched on or off, the answer to the question about the time left.
    /// @param on True to switch the body shift on.
    /// @return The encoded request payload.
    core::Bytes bodyShiftToggleRequest(bool on);

    /// @brief Builds request 23/114 sub 1: a Riceball is used.
    /// @details Original: SendBodyShiftRestart. Layout: u8 bagSlot.
    /// @param bagSlot Bag slot of the Riceball.
    /// @return The encoded request payload.
    core::Bytes bodyShiftUseRequest(int bagSlot);

    /// @brief Builds request 23/114 sub 2: asks the time left of the body shift.
    /// @details Original: UseChangeBodyItem.
    /// @return The encoded request payload.
    core::Bytes bodyShiftAskRequest();

}

#endif
