#include "protocol/TentRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        /// @brief Narrows an int to a single protocol byte.
        /// @param value Value to narrow.
        /// @return The low 8 bits of the value.
        std::uint8_t byte(int value) { return static_cast<std::uint8_t>(value); }

    }

    core::Bytes enterTentRequest(std::int32_t ownerId) {
        core::ByteWriter writer;
        writer.u8(65).u8(1).i32(ownerId); // 65 = tent action, 1 = enter tent sub id
        return writer.take();
    }

    core::Bytes takeBackTentRequest(int bagSlot) { return {65, 2, byte(bagSlot)}; } // 65 = tent action, 2 = take back sub id

    core::Bytes tentPaletteRequest(int bagSlot, int set) { return {23, 94, byte(bagSlot), byte(set)}; } // 23 = item action, 94 = change camp sub id

    core::Bytes tentStyleRequest(int style, int bagSlot, int set) {
        // 65: tent action, 12: confirm style sub id
        return {65, 12, byte(style), byte(bagSlot), byte(set)};
    } // 65 = tent action, 12 = confirm style sub id

}
