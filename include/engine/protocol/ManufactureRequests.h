#ifndef NORTH_ISLAND_PROTOCOL_MANUFACTURE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_MANUFACTURE_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds 64/1: start making a recipe at a piece of furniture.
    /// @details The blueprint source is 1 the bag, 2 the bank, 3 a book case, 4 a book desk (0: the recipe needs none).
    /// @param piece Piece number (u16).
    /// @param recipe Recipe (u16).
    /// @param materials The materials (MakeCheck::request).
    /// @param blueprintSource Blueprint source (u8).
    /// @param blueprintSlot Blueprint place (u8).
    /// @param petSlot Pet place (u8).
    /// @param petId Pet id (i32).
    /// @return The encoded payload.
    core::Bytes startMakingRequest(
        int piece,
        int recipe,
        const core::Bytes& materials,
        int blueprintSource,
        int blueprintSlot,
        int petSlot = 0,
        int petId = 0
    );

    /// @brief Builds 64/2: go on with a stopped job.
    /// @param piece Piece number (u16).
    /// @param jobSlot Job slot (u8).
    /// @param blueprintSource Blueprint source (u8).
    /// @param blueprintSlot Blueprint place (u8).
    /// @param petSlot Pet place (u8).
    /// @param petId Pet id (i32).
    /// @return The encoded payload.
    core::Bytes continueMakingRequest(
        int piece,
        int jobSlot,
        int blueprintSource,
        int blueprintSlot,
        int petSlot = 0,
        int petId = 0
    );

    /// @brief Builds 64/3: stop a job.
    /// @param jobSlot Job slot (u8).
    /// @return The encoded payload.
    core::Bytes stopMakingRequest(int jobSlot);

    /// @brief Builds 64/4: give a job up.
    /// @param jobSlot Job slot (u8).
    /// @return The encoded payload.
    core::Bytes giveUpMakingRequest(int jobSlot);

}

#endif
