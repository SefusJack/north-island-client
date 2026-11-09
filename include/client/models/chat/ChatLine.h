#ifndef NORTH_ISLAND_CLIENT_MODELS_CHAT_CHAT_LINE_H
#define NORTH_ISLAND_CLIENT_MODELS_CHAT_CHAT_LINE_H

#include "core/Color.h"

#include <cstdint>
#include <string>

namespace client {

    /// @brief One line of the chat window as it is drawn (a long message takes several).
    /// @details Emote codes are blanked in the text; emotes marks them: per column a space, or 32 + the emote's number over its code (FLineLinks3).
    struct ChatLine {
        std::string text;
        std::string emotes;
        core::Color color;
        int channel = 0;
        std::int32_t senderId = 0;
    };

}

#endif
