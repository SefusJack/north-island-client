#ifndef NORTH_ISLAND_PROTOCOL_EMOTION_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_EMOTION_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds client 32/1: show an emotion over the own head to the others.
    /// @param emotionId Emotion (u8, 101..124).
    /// @return The encoded request.
    core::Bytes emotionRequest(int emotionId);

    /// @brief Builds client 32/2: the own character plays the action in that sprite slot.
    /// @param slot Sprite slot (u8).
    /// @return The encoded request.
    core::Bytes actionRequest(int slot);

    /// @brief Builds client 32/3: the own character's action ended.
    /// @return The encoded request.
    core::Bytes stopActionRequest();

}

#endif
