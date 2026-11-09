#ifndef NORTH_ISLAND_PROTOCOL_WISH_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_WISH_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds request 23/129: the Bliss Lamp message.
    /// @details Original: TEnterMsgForm2.SendBlessMessage. Layout: u8 bag place, i32 player, u8 length of the trimmed text, the text.
    /// @param bagSlot Bag place of the lamp.
    /// @param playerId Player id receiving the blessing.
    /// @param length Length of the trimmed text.
    /// @param text Message text.
    /// @return The encoded request payload.
    core::Bytes blessMessageRequest(int bagSlot, std::int32_t playerId, int length, const std::string& text);

    /// @brief Builds request 23/118: the Passion Bouquet message, laid out like the Bliss Lamp one.
    /// @details Original: TEnterMsgForm.SendFlowerMessage.
    /// @param bagSlot Bag place of the bouquet.
    /// @param playerId Player id receiving the flower.
    /// @param length Length of the trimmed text.
    /// @param text Message text.
    /// @return The encoded request payload.
    core::Bytes flowerMessageRequest(int bagSlot, std::int32_t playerId, int length, const std::string& text);

}

#endif
