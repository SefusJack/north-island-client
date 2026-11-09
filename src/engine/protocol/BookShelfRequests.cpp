#include "protocol/BookShelfRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kStatus = 5; // action 5: player status
        constexpr std::uint8_t kBookcasePut = 9; // 5/9: put a book in the bookcase
        constexpr std::uint8_t kBookcaseTake = 10; // 5/10: take a book from the bookcase
        constexpr std::uint8_t kDeskPut = 11; // 5/11: put a book on the desk
        constexpr std::uint8_t kDeskTake = 12; // 5/12: take a book from the desk

    }

    core::Bytes putInBookShelfRequest(bool desk, int bagSlot, int place) {
        return {kStatus, desk ? kDeskPut : kBookcasePut, static_cast<std::uint8_t>(bagSlot), static_cast<std::uint8_t>(place)};
    }

    core::Bytes takeFromBookShelfRequest(bool desk, int place) { return {kStatus, desk ? kDeskTake : kBookcaseTake, static_cast<std::uint8_t>(place)}; }

}
