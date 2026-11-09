#ifndef NORTH_ISLAND_PROTOCOL_WORLD_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_WORLD_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 20/8: the own player walked into a door of the map (u16 door).
    /// @param doorIndex The door's eve.Emg index.
    /// @return The encoded request payload.
    core::Bytes doorEnteredRequest(int doorIndex);

    /// @brief Builds request 20/1: the own player clicked an NPC with events (u16 NPC).
    /// @param npcIndex The NPC's eve.Emg index.
    /// @return The encoded request payload.
    core::Bytes npcClickedRequest(int npcIndex);

    /// @brief Builds request 20/2: the own player came near an NPC whose events start so (u16 NPC).
    /// @param npcIndex The NPC's eve.Emg index.
    /// @return The encoded request payload.
    core::Bytes npcNearRequest(int npcIndex);

    /// @brief Builds request 20/9: the answer to an event's dialog (u8 answer).
    /// @param answer Confirm 20, Cancel 21, choice n 30 + n, Close 40.
    /// @return The encoded request payload.
    core::Bytes eventAnswerRequest(int answer);

    /// @brief Builds request 20/6: asks for the next result of the running event.
    /// @return The encoded request payload.
    core::Bytes nextEventResultRequest();

    /// @brief Builds request 12/1: the new map is loaded (after server 12).
    /// @return The encoded request payload.
    core::Bytes mapLoadedRequest();

}

#endif
