#include "protocol/RemoteRequests.h"

#include "core/ByteWriter.h"

#include <algorithm>

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kSystem = 5;

        /// @brief Starts an action 23 packet with its sub code.
        /// @param sub Sub code.
        /// @return A writer holding action and sub.
        core::ByteWriter itemsMessage(std::uint8_t sub) {
            core::ByteWriter writer;
            writer.u8(kItems).u8(sub);
            return writer;
        }

    }

    core::Bytes remoteOpenRequest(int bagSlot) {
        core::ByteWriter writer = itemsMessage(76); // sub 76: Remote Control used
        writer.u8(static_cast<std::uint8_t>(bagSlot));
        return writer.take();
    }

    core::Bytes remoteStartRequest(
        int ticks,
        int hpPercent,
        int spPercent,
        int petHpPercent,
        int petSpPercent,
        int deathLimit,
        int petDeathLimit,
        int leaveMinutes,
        int itemFrom,
        int itemTo
    ) {
        // 65535: largest u16
        const auto word = [](int value) { return static_cast<std::uint16_t>(std::min(value, 65535)); };
        core::ByteWriter writer = itemsMessage(78); // sub 78: Remote Control settings
        writer
            .u16(static_cast<std::uint16_t>(ticks))
            // 100000: the player's percent is packed above the pet's percent
            .i32(hpPercent * 100000 + petHpPercent)
            .i32(spPercent * 100000 + petSpPercent)
            .u16(word(deathLimit))
            .u16(word(petDeathLimit))
            .u16(word(leaveMinutes))
            .u8(static_cast<std::uint8_t>(itemFrom))
            .u8(static_cast<std::uint8_t>(itemTo));
        return writer.take();
    }

    core::Bytes remoteStopRequest() { return itemsMessage(79).take(); } // sub 79: stopped by the player

    core::Bytes remoteGaveUpRequest() { return itemsMessage(81).take(); } // sub 81: gave up, no supply left

    core::Bytes remoteLeaveRequest() {
        core::ByteWriter writer;
        writer.u8(kSystem).u8(8); // action 5, sub 8: AFK auto stop, leaving the game
        return writer.take();
    }

}
