#ifndef NORTH_ISLAND_PROTOCOL_CASTLE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_CASTLE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 70/3: a click on the Leave battle icon of the time effects (original HandleTimeEffectClick 6).
    /// @return The encoded request.
    core::Bytes trojanWarLeaveRequest();

    /// @brief Builds client 70/7: the castle's guild member goes to the castle once the teleport light has played.
    /// @details Original: the GPS's 74/2 of the guild window's Btn_Transmi.
    /// @return The encoded request.
    core::Bytes castleTeleportRequest();

    /// @brief Builds client 70/4: the disguise flag is cleared outside the battle maps (original UpdateTimeEffectIcons).
    /// @return The encoded request.
    core::Bytes trojanWarFlagResetRequest();

}

#endif
