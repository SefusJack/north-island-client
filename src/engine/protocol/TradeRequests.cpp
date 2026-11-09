#include "protocol/TradeRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kTrade = 25; // action 25: trade

    }

    core::Bytes itemTradeRequest(std::int32_t playerId) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(1).i32(playerId); // 1 = ask item trade sub id
        return writer.take();
    }

    core::Bytes tradeOfferRequest(std::int32_t gold, const std::vector<int>& bagSlots) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(2).i32(gold); // 2 = item offer sub id
        for (const int slot : bagSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        return writer.take();
    }

    core::Bytes tradeAnswerRequest(int answer) {
        core::ByteWriter writer;
        // 3 = item trade answer sub id
        writer.u8(kTrade).u8(3).u8(static_cast<std::uint8_t>(answer));
        return writer.take();
    }

    core::Bytes petTradeRequest(std::int32_t playerId) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(10).i32(playerId); // 10 = ask pet trade sub id
        return writer.take();
    }

    core::Bytes petTradeOfferRequest(std::int32_t gold, int petPlace) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(11).i32(gold).u8(static_cast<std::uint8_t>(petPlace)); // 11 = pet offer sub id
        return writer.take();
    }

    core::Bytes petTradeAnswerRequest(int answer) {
        core::ByteWriter writer;
        // 12 = pet trade answer sub id
        writer.u8(kTrade).u8(12).u8(static_cast<std::uint8_t>(answer));
        return writer.take();
    }

}
