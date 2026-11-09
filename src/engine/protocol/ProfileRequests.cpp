#include "protocol/ProfileRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        /// @brief Narrows an int to a payload byte.
        /// @param value The value.
        /// @return The low byte.
        std::uint8_t byte(int value) { return static_cast<std::uint8_t>(value); }

    }

    core::Bytes onlineStatusRequest(int status) {
        core::ByteWriter writer;
        writer.u8(5).u8(7).u8(byte(status)); // action 5, sub 7: online status
        return writer.take();
    }

    core::Bytes nicknameRequest(const std::string& nickname) {
        core::ByteWriter writer;
        writer.u8(10).u8(1).u8(byte(static_cast<int>(nickname.size()))).text(nickname); // action 10, sub 1: nickname
        return writer.take();
    }

    core::Bytes profileRequest(
        int heartNumber,
        int bloodType,
        int birthYear,
        int birthMonth,
        int birthDay
    ) {
        core::ByteWriter writer;
        // action 10, sub 2: profile
        writer.u8(10).u8(2).u8(byte(heartNumber)).u8(byte(bloodType)).u8(byte(birthYear)).u8(byte(birthMonth)).u8(byte(birthDay));
        return writer.take();
    }

    core::Bytes profileDoneRequest() {
        core::ByteWriter writer;
        writer.u8(10).u8(3); // action 10, sub 3: profile handed in
        return writer.take();
    }

}
