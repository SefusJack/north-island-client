#ifndef NORTH_ISLAND_PROTOCOL_CAREER_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_CAREER_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 79/1: the class chosen at the rebirth (original SendChooseCareer, DZ_Connect.pas).
    /// @param career Career (u8).
    /// @return The encoded request.
    core::Bytes chooseCareerRequest(int career);

    /// @brief Builds client 79/2: a reborn player's class changed (original SendChangeCareer).
    /// @param career Career (u8).
    /// @param mirrorSlot Bag place (u8) of the Transfer Mirror.
    /// @param cloakSlot Bag place (u8) of the class cloak.
    /// @return The encoded request.
    core::Bytes changeCareerRequest(int career, int mirrorSlot, int cloakSlot);

}

#endif
