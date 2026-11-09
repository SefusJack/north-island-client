#ifndef NORTH_ISLAND_PROTOCOL_SANDGLASS_MESSAGES_H
#define NORTH_ISLAND_PROTOCOL_SANDGLASS_MESSAGES_H

#include "core/Bytes.h"

#include <optional>

namespace protocol {

    /// @brief Builds the client request 62/53 sub 1 saying the owner agreed to use a sandglass piece.
    /// @details Original: ConfirmUseSandglass sends FSandglassUsed and FSandglassFurnIndex.
    /// @param piece Index of the sandglass piece being used.
    /// @return The encoded request payload.
    core::Bytes sandglassRequest(int piece);

    /// @brief Server report of how much of a sandglass piece is used up (S->C 62/53 sub 1).
    /// @details Original: HandleSandglassUsedPacket; value is out of 250.
    struct SandglassUsed {
        int piece = 0;
        int value = 0;
    };

    /// @brief Decodes S->C 62/53 sub 1 (piece and used-up value).
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The decoded record, or nullopt when the sub id differs or the payload is truncated.
    std::optional<SandglassUsed> decodeSandglassUsed(const core::Bytes& payload);

    /// @brief Decodes S->C 62/53 sub 2 (job flags).
    /// @details Original: HandleSandglassEffectPacket; bit n-1 marks the job of slot n running under a sandglass, 32 marks a use no job has taken yet.
    /// @param payload Packet payload including the action and sub id bytes.
    /// @return The flag byte, or nullopt when the sub id differs or the payload is truncated.
    std::optional<int> decodeSandglassFlags(const core::Bytes& payload);

}

#endif
