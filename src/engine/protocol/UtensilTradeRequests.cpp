#include "protocol/UtensilTradeRequests.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kTrade = 25; // action 25: trade
        constexpr std::uint8_t kRequest = 40; // 25/40: ask for a utensil trade
        constexpr std::uint8_t kOffer = 41; // 25/41: offer pieces
        constexpr std::uint8_t kAnswer = 42; // 25/42: answer the utensil trade
        constexpr std::size_t kItemRecord = 9;

    }

    core::Bytes utensilTradeRequest(std::int32_t playerId) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(kRequest).i32(playerId);
        return writer.take();
    }

    core::Bytes utensilOfferRequest(std::int32_t gold, const std::vector<int>& bagSlots) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(kOffer).i32(gold);
        for (const int slot : bagSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        return writer.take();
    }

    core::Bytes utensilAnswerRequest(int answer) {
        core::ByteWriter writer;
        writer.u8(kTrade).u8(kAnswer).u8(static_cast<std::uint8_t>(answer));
        return writer.take();
    }

    std::optional<TradeOffer> decodeUtensilOffer(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        TradeOffer offer;
        offer.gold = reader.i32();
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() >= kItemRecord) {
            ItemInstance item;
            item.itemId = reader.u16();
            item.count = 1;
            item.usedAmount = reader.i32();
            item.extraAmount = reader.u16();
            item.wear = reader.u8();
            offer.items.push_back(item);
        }
        return offer;
    }

}
