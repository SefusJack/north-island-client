#include "protocol/WishRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kFlowerMessage = 118; // 23/118: flower message
        constexpr std::uint8_t kBlessMessage = 129; // 23/129: blessing message

        /// @brief Builds a 23 message request: u8 bag place, i32 player, u8 length, the text.
        /// @param sub Sub id (bless 129 or flower 118).
        /// @param bagSlot Bag place of the item.
        /// @param playerId Player id receiving the message.
        /// @param length Length of the trimmed text.
        /// @param text Message text.
        /// @return The encoded request payload.
        core::Bytes messageRequest(
            std::uint8_t sub,
            int bagSlot,
            std::int32_t playerId,
            int length,
            const std::string& text
        ) {
            core::ByteWriter writer;
            writer.u8(kItems).u8(sub).u8(static_cast<std::uint8_t>(bagSlot)).i32(playerId).u8(static_cast<std::uint8_t>(length));
            writer.text(text);
            return writer.take();
        }

    }

    core::Bytes blessMessageRequest(int bagSlot, std::int32_t playerId, int length, const std::string& text) {
        return messageRequest(kBlessMessage, bagSlot, playerId, length, text);
    }

    core::Bytes flowerMessageRequest(int bagSlot, std::int32_t playerId, int length, const std::string& text) {
        return messageRequest(kFlowerMessage, bagSlot, playerId, length, text);
    }

}
