#include "protocol/FurnitureStallRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture

        /// @brief Builds a client 62/sub message that puts bag items on a stall (u16 piece, u16 item, u8 bag place, u8 count, i32 price).
        /// @param piece Furniture piece.
        /// @param itemId Item.
        /// @param bagSlot Bag place.
        /// @param count How many.
        /// @param price Price.
        /// @param sub Sub id (21 shelf, 24 vending machine).
        /// @return A writer holding the message.
        core::ByteWriter put(
            int piece,
            int itemId,
            int bagSlot,
            int count,
            std::int32_t price,
            std::uint8_t sub
        ) {
            core::ByteWriter writer;
            writer.u8(kHouse).u8(sub).u16(static_cast<std::uint16_t>(piece)).u16(static_cast<std::uint16_t>(itemId));
            writer.u8(static_cast<std::uint8_t>(bagSlot)).u8(static_cast<std::uint8_t>(count)).i32(price);
            return writer;
        }

        /// @brief Builds a client 62/sub buy message (i32 owner, u16 stall number, u8 count, u16 echoed item, i32 echoed price).
        /// @param ownerId Owner's player id.
        /// @param stallSlot Stall number.
        /// @param count How many.
        /// @param echoItem The item word of the item's record.
        /// @param echoPrice The price integer of the item's record.
        /// @param sub Sub id (23 shelf, 26 vending machine).
        /// @return A writer holding the message.
        core::ByteWriter buy(
            std::int32_t ownerId,
            int stallSlot,
            int count,
            int echoItem,
            std::int32_t echoPrice,
            std::uint8_t sub
        ) {
            core::ByteWriter writer;
            writer.u8(kHouse).u8(sub).i32(ownerId).u16(static_cast<std::uint16_t>(stallSlot)).u8(static_cast<std::uint8_t>(count));
            writer.u16(static_cast<std::uint16_t>(echoItem)).i32(echoPrice);
            return writer;
        }

    }

    core::Bytes putOnShelfRequest(
        int piece,
        int itemId,
        int bagSlot,
        int count,
        std::int32_t price
    ) {
        return put(piece, itemId, bagSlot, count, price, 21).take(); // 62/21: put on a shelf
    }

    core::Bytes putInVendingRequest(
        int piece,
        int itemId,
        int bagSlot,
        int count,
        std::int32_t price,
        int row
    ) {
        core::ByteWriter writer = put(piece, itemId, bagSlot, count, price, 24); // 62/24: put in a vending machine
        writer.u8(static_cast<std::uint8_t>(row));
        return writer.take();
    }

    core::Bytes takeBackFromShelfRequest(int stallSlot) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(22).u16(static_cast<std::uint16_t>(stallSlot)); // 62/22: take back from a shelf
        return writer.take();
    }

    core::Bytes takeBackFromVendingRequest(int stallSlot) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(25).u16(static_cast<std::uint16_t>(stallSlot)); // 62/25: take back from a vending machine
        return writer.take();
    }

    core::Bytes buyFromShelfRequest(
        std::int32_t ownerId,
        int stallSlot,
        int count,
        int echoItem,
        std::int32_t echoPrice
    ) {
        return buy(ownerId, stallSlot, count, echoItem, echoPrice, 23).take(); // 62/23: buy from a shelf
    }

    core::Bytes buyFromVendingRequest(
        std::int32_t ownerId,
        int stallSlot,
        int count,
        int echoItem,
        std::int32_t echoPrice
    ) {
        return buy(ownerId, stallSlot, count, echoItem, echoPrice, 26).take(); // 62/26: buy from a vending machine
    }

}
