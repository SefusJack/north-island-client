#include "protocol/CafeRequests.h"

namespace protocol {

    core::Bytes cafeNumberRequest(const std::string& number) {
        constexpr std::uint8_t kCafeAction = 55; // action 55: internet cafe
        constexpr std::uint8_t kNumberSub = 1; // 55/1: send the cafe number
        core::Bytes request{kCafeAction, kNumberSub};
        request.insert(request.end(), number.begin(), number.end());
        return request;
    }

}
