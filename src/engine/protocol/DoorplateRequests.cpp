#include "protocol/DoorplateRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture
        constexpr std::uint8_t kClear = 45; // 62/45: clear the doorplate text
        constexpr std::uint8_t kSet = 46; // 62/46: set the doorplate text

    }

    core::Bytes doorplateRequest(const std::string& text) {
        if (text.empty()) {
            return {kHouse, kClear};
        }
        core::Bytes message{kHouse, kSet};
        message.insert(message.end(), text.begin(), text.end());
        return message;
    }

}
