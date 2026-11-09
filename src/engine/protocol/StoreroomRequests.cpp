#include "protocol/StoreroomRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture

        /// @brief Narrows an int to a single protocol byte.
        /// @param value Value to narrow.
        /// @return The low 8 bits of the value.
        std::uint8_t byte(int value) { return static_cast<std::uint8_t>(value); }

        /// @brief Builds a 62 request of the shape u8 first, u8 second, u8 count, u16 piece.
        /// @param sub House sub id.
        /// @param first First byte argument.
        /// @param second Second byte argument.
        /// @param count Item count byte.
        /// @param piece Furniture index of the Storeroom piece.
        /// @return The encoded request payload.
        core::Bytes threeBytes(
            std::uint8_t sub,
            int first,
            int second,
            int count,
            int piece
        ) {
            core::ByteWriter writer;
            writer.u8(kHouse).u8(sub).u8(byte(first)).u8(byte(second)).u8(byte(count)).u16(static_cast<std::uint16_t>(piece));
            return writer.take();
        }

    }

    core::Bytes storeroomPutRequest(int bagSlot, int cell, int count, int piece) {
        return threeBytes(7, bagSlot, cell, count, piece);
    } // 7 = bag to Storeroom sub id

    core::Bytes storeroomMoveRequest(int from, int to, int count, int piece) {
        return threeBytes(8, from, to, count, piece);
    } // 8 = move within Storeroom sub id

    core::Bytes storeroomTakeRequest(int cell, int bagSlot, int count, int piece) {
        return threeBytes(9, cell, bagSlot, count, piece);
    } // 9 = Storeroom to bag sub id

    core::Bytes storeroomRecycleRequest(int cell, int count, int bin, int piece) {
        core::ByteWriter writer;
        // 12 = recycle from Storeroom sub id
        writer.u8(kHouse).u8(12).u8(byte(cell)).u8(byte(count)).u16(static_cast<std::uint16_t>(bin)).u16(static_cast<std::uint16_t>(piece));
        return writer.take();
    }

    core::Bytes superHandbagRequest() { return {kHouse, 50}; } // 50 = Super Handbag sub id

}
