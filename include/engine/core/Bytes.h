#ifndef NORTH_ISLAND_CORE_BYTES_H
#define NORTH_ISLAND_CORE_BYTES_H

#include <cstdint>
#include <string>
#include <vector>

namespace core {

    using Bytes = std::vector<std::uint8_t>;

    /// @brief Copies a string's characters into a byte block.
    /// @param text The text.
    /// @return The bytes.
    inline Bytes toBytes(const std::string& text) { return Bytes(text.begin(), text.end()); }

    /// @brief Copies a byte block into a string.
    /// @param bytes The bytes.
    /// @return The text.
    inline std::string toString(const Bytes& bytes) { return std::string(bytes.begin(), bytes.end()); }

}

#endif
