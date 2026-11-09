#ifndef NORTH_ISLAND_PROTOCOL_CHAT_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_CHAT_REQUEST_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds client 2/sub: a chat line (text without length byte).
    /// @param sub 1 world, 2 local, 4 GM, 5 team, 6 guild, 7 ally.
    /// @param text The chat text.
    /// @return The encoded request.
    core::Bytes chatRequest(int sub, const std::string& text);

    /// @brief Builds client 2/3: a whisper; the i32 target id comes before the text.
    /// @param targetId Player id of the target.
    /// @param text The chat text.
    /// @return The encoded request.
    core::Bytes whisperRequest(std::int32_t targetId, const std::string& text);

}

#endif
