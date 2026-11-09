#include "protocol/CareerRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kCareer = 79; // action 79: career

        /// @brief Narrows a value to a byte.
        /// @param value The value.
        /// @return Its low byte.
        std::uint8_t byte(int value) { return static_cast<std::uint8_t>(value); }

    }

    core::Bytes chooseCareerRequest(int career) { return {kCareer, 1, byte(career)}; } // 79/1: choose a career

    core::Bytes changeCareerRequest(int career, int mirrorSlot, int cloakSlot) {
        return {kCareer, 2, byte(career), byte(mirrorSlot), byte(cloakSlot)}; // 79/2: change career
    }

}
