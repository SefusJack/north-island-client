#include "protocol/BankRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kBankGold = 29; // action 29: bank / ATM gold
        constexpr std::uint8_t kBankItems = 30; // action 30: bank items

        /// @brief Builds a client 30/sub request made of one u8 place per slot.
        /// @param sub Sub id.
        /// @param slots Places, one byte each.
        /// @return The encoded request.
        core::Bytes slotsRequest(std::uint8_t sub, const std::vector<int>& slots) {
            core::ByteWriter writer;
            writer.u8(kBankItems).u8(sub);
            for (const int slot : slots) {
                writer.u8(static_cast<std::uint8_t>(slot));
            }
            return writer.take();
        }

    }

    core::Bytes bankWithdrawRequest(const std::vector<int>& bankSlots) { return slotsRequest(1, bankSlots); } // 30/1: withdraw marked items

    core::Bytes bankStoreRequest(const std::vector<int>& bagSlots) { return slotsRequest(2, bagSlots); } // 30/2: store marked items

    core::Bytes bankMoveRequest(int fromSlot, int toSlot) { return slotsRequest(3, {fromSlot, toSlot}); } // 30/3: move a bank item

    core::Bytes bankDropRequest(int bankSlot, int bagSlot) { return slotsRequest(4, {bankSlot, bagSlot}); } // 30/4: drop a bag item on the bank

    core::Bytes bankTakeRequest(int bagSlot, int bankSlot) { return slotsRequest(5, {bagSlot, bankSlot}); } // 30/5: drop a bank item on the bag

    core::Bytes bankGoldWithdrawRequest(std::int32_t amount) {
        core::ByteWriter writer;
        writer.u8(kBankGold).u8(1).i32(amount); // 29/1: withdraw gold
        return writer.take();
    }

    core::Bytes bankGoldDepositRequest(std::int32_t amount) {
        core::ByteWriter writer;
        writer.u8(kBankGold).u8(2).i32(amount); // 29/2: deposit gold
        return writer.take();
    }

    core::Bytes atmDepositRequest(std::int32_t amount) {
        core::ByteWriter writer;
        writer.u8(kBankGold).u8(3).i32(amount); // 29/3: carried gold into the ATM
        return writer.take();
    }

    core::Bytes atmWithdrawRequest(std::int32_t amount) {
        core::ByteWriter writer;
        writer.u8(kBankGold).u8(4).i32(amount); // 29/4: ATM gold into carried gold
        return writer.take();
    }

    core::Bytes atmTransferRequest(std::int32_t playerId, std::int32_t amount) {
        core::ByteWriter writer;
        writer.u8(kBankGold).u8(5).i32(playerId).i32(amount); // 29/5: ATM transfer to another player
        return writer.take();
    }

}
