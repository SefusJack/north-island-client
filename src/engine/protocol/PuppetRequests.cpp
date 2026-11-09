#include "protocol/PuppetRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kPuppet = 104; // 23/104: puppet

        /// @brief Starts a 23/104 packet with its option number.
        /// @param number Option number.
        /// @return A writer holding action, sub and option.
        core::ByteWriter option(std::uint8_t number) {
            core::ByteWriter writer;
            writer.u8(kItems).u8(kPuppet).u8(number);
            return writer;
        }

    }

    core::Bytes puppetInviteRequest(std::int32_t helperId, int bagSlot) {
        core::ByteWriter writer = option(1); // option 1: invite a helper
        writer.i32(helperId).u8(static_cast<std::uint8_t>(bagSlot));
        return writer.take();
    }

    core::Bytes puppetAcceptRequest(std::int32_t requesterId) {
        core::ByteWriter writer = option(2); // option 2: the helper agrees
        writer.i32(requesterId);
        return writer.take();
    }

    core::Bytes puppetRefuseRequest(int reason, std::int32_t requesterId) {
        core::ByteWriter writer = option(3); // option 3: the helper refuses
        writer.u8(static_cast<std::uint8_t>(reason)).i32(requesterId);
        return writer.take();
    }

    core::Bytes puppetKeepRequest(int reason) {
        core::ByteWriter writer = option(4); // option 4: the requester refuses to stop
        writer.u8(static_cast<std::uint8_t>(reason));
        return writer.take();
    }

    core::Bytes puppetAskStopRequest() { return option(5).take(); } // option 5: the requester asks to stop

    core::Bytes puppetHelperStopRequest(std::int32_t requesterId) {
        core::ByteWriter writer = option(6); // option 6: the helper agrees to stop
        writer.i32(requesterId);
        return writer.take();
    }

    core::Bytes puppetUseExpRequest() { return option(7).take(); } // option 7: the requester spends the card's exp

    core::Bytes puppetHelperAskStopRequest(std::int32_t requesterId) {
        core::ByteWriter writer = option(8); // option 8: the helper asks to stop
        writer.i32(requesterId);
        return writer.take();
    }

    core::Bytes puppetAgreeStopRequest() { return option(9).take(); } // option 9: the requester agrees to stop

    core::Bytes puppetHelperKeepRequest(int reason, std::int32_t requesterId) {
        core::ByteWriter writer = option(10); // option 10: the helper refuses to stop
        writer.u8(static_cast<std::uint8_t>(reason)).i32(requesterId);
        return writer.take();
    }

    core::Bytes puppetOpenRequest() { return option(12).take(); } // option 12: open the requester's form (answer is 14)

    core::Bytes puppetMailRequest(std::int32_t ownId, const std::string& partnerName, const std::string& text) {
        core::ByteWriter writer = option(13); // option 13: mail to the partner
        writer.i32(ownId).shortText(partnerName);
        writer.text(text);
        return writer.take();
    }

    core::Bytes puppetTimeLeftRequest(int slot) {
        core::ByteWriter writer = option(14); // option 14: time left of a card
        writer.u8(static_cast<std::uint8_t>(slot));
        return writer.take();
    }

    core::Bytes puppetGrowRequest() { return option(15).take(); } // option 15: grow the card by one step

}
