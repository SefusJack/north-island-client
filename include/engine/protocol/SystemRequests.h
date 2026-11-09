#ifndef NORTH_ISLAND_PROTOCOL_SYSTEM_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_SYSTEM_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds a system-window switch request: 33/1 PK, 33/2 fighting, 33/4 trade, 33/5 the fourth switch.
    /// @param sub Sub id of the switch (1, 2, 4 or 5).
    /// @param state 1 for on, 2 for off.
    /// @return The encoded request payload.
    core::Bytes switchRequest(int sub, int state);

    /// @brief Builds request 23/77: the chat box is set up.
    /// @details Original: SendTalkFormReady.
    /// @return The encoded request payload.
    core::Bytes talkFormReadyRequest();

    /// @brief Builds request 33/3: the chat channels that are on.
    /// @param bits Channel bits: bit 0 local, 1 whisper, 2 team, 3 party, 4 world.
    /// @return The encoded request payload.
    core::Bytes channelsRequest(int bits);

    /// @brief Builds request 5/17: go back to a revival spot.
    /// @param spot 1 the starter's beach, 2 the recorded spot, 3 the carnival.
    /// @return The encoded request payload.
    core::Bytes returnToSpotRequest(int spot);

    /// @brief Builds request 34/1 (u8 0): asks for the account's balance.
    /// @return The encoded request payload.
    core::Bytes balanceRequest();

    /// @brief Builds request 23/52, sent before leaving the world to stop the express.
    /// @return The encoded request payload.
    core::Bytes stopExpressRequest();

    /// @brief Builds request 23/52: the player left the water.
    /// @details Original: ForceLeaveWater, UpdateInWaterState.
    /// @return The encoded request payload.
    core::Bytes leaveWaterRequest();

    /// @brief Builds request 23/54, sent before leaving the world to stop collecting.
    /// @return The encoded request payload.
    core::Bytes stopCollectingRequest();

}

#endif
