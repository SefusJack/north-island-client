#include "protocol/BoxRequests.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kPicnicSub = 131; // 23/131: picnic box
        constexpr std::uint8_t kExpBagSub = 132; // 23/132: experience bag

        /// @brief Narrows a value to a byte.
        /// @param value The value.
        /// @return Its low byte.
        std::uint8_t byteOf(int value) { return static_cast<std::uint8_t>(value); }

    }

    core::Bytes boxPutRequest(bool picnic, int bagSlot, int cell, int boxSlot) {
        return {kItems, picnic ? kPicnicSub : kExpBagSub, 1, byteOf(bagSlot), byteOf(cell), 0, byteOf(boxSlot)};
    }

    core::Bytes boxTakeRequest(bool picnic, int cell, int boxSlot) {
        return {kItems, picnic ? kPicnicSub : kExpBagSub, 2, byteOf(cell), 0, byteOf(boxSlot)};
    } // op 2: take back

    core::Bytes boxUseRequest(bool picnic, int cell, int target, int boxSlot) {
        return {kItems, picnic ? kPicnicSub : kExpBagSub, 3, byteOf(cell), byteOf(target), byteOf(boxSlot)}; // op 3: use one
    }

}
