#ifndef NORTH_ISLAND_PROTOCOL_FRAMING_H
#define NORTH_ISLAND_PROTOCOL_FRAMING_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    constexpr std::uint8_t kFrameMagic0 = 0xF4; // first byte of every frame header
    constexpr std::uint8_t kFrameMagic1 = 0x44; // second byte of every frame header
    constexpr std::uint8_t kStreamKey = 0xAD; // key byte of the stream cipher applied to payloads

    /// @brief Wraps a payload in a frame: F4 44, u16 payload length, payload.
    /// @param payload Message payload.
    /// @return The framed bytes.
    core::Bytes frame(const core::Bytes& payload);

    /// @brief XORs a byte stream with the stream key 0xAD (same for both directions).
    /// @param bytes Bytes changed in place.
    void applyStreamKey(core::Bytes& bytes);

}

#endif
