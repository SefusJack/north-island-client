#include "protocol/GiftCardRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes giftCardUseRequest(int bagSlot, int cardId, int version) {
        core::ByteWriter writer;
        writer
            .u8(23) // action 23: items
            .u8(133) // sub 133: gift card
            .u8(1) // option 1: use the card
            .u8(static_cast<std::uint8_t>(bagSlot))
            .u16(static_cast<std::uint16_t>(cardId))
            .u8(static_cast<std::uint8_t>(version)); // 23/133/1: card used
        return writer.take();
    }

    core::Bytes giftCardExchangeRequest(int bagSlot, int times, int choice, int itemId) {
        core::ByteWriter writer;
        writer
            .u8(23) // action 23: items
            .u8(133) // sub 133: gift card
            .u8(2) // option 2: exchange the card
            .u8(static_cast<std::uint8_t>(bagSlot))
            .u8(static_cast<std::uint8_t>(times))
            .u8(static_cast<std::uint8_t>(choice)); // 23/133/2: card exchanged
        writer.u16(static_cast<std::uint16_t>(itemId));
        return writer.take();
    }

    core::Bytes giftCardListRequest(const std::vector<int>& cards) {
        core::ByteWriter writer;
        writer.u8(90).u8(1).u8(1).u16(static_cast<std::uint16_t>(cards.size())); // 90/1/1: ask for card lists
        for (const int card : cards) {
            writer.u16(static_cast<std::uint16_t>(card));
        }
        return writer.take();
    }

}
