#include "protocol/PotentialRequests.h"

namespace protocol {

    core::Bytes potentialRequest(int bagSlot, int petPlace) {
        return {23, 126, static_cast<std::uint8_t>(bagSlot), static_cast<std::uint8_t>(petPlace)}; // action 23, sub 126: Potential Pill
    }

    core::Bytes potentialReturnRequest(int bagSlot, int petPlace) {
        return {23, 130, static_cast<std::uint8_t>(bagSlot), static_cast<std::uint8_t>(petPlace)}; // action 23, sub 130: Anti-Potential Pill
    }

}
