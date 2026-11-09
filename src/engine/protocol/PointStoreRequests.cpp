#include "protocol/PointStoreRequests.h"

namespace protocol {

    core::Bytes pointStoreBuyRequest(const std::vector<std::pair<int, int>>& lines) {
        core::Bytes bytes{80, 1, static_cast<std::uint8_t>(lines.size())}; // action 80 (point store), sub 1: buy the cart
        for (const auto& [row, count] : lines) {
            bytes.push_back(static_cast<std::uint8_t>(row));
            bytes.push_back(static_cast<std::uint8_t>(count));
        }
        return bytes;
    }

    core::Bytes pointStoreLevelRequest(bool pet) {
        return core::Bytes{80, static_cast<std::uint8_t>(pet ? 3 : 2)}; // action 80: point store
    } // action 80, sub 3: pet level, 2: player level

}
