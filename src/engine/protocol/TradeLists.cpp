#include "protocol/TradeLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kItemRecord = 28; // bytes per offered item record

    }

    std::optional<TradeOffer> decodeTradeOffer(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        TradeOffer offer;
        offer.gold = reader.i32();
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() >= kItemRecord) {
            ItemInstance item;
            item.itemId = reader.u16();
            item.count = reader.u8();
            item.wear = reader.u8();
            item.usedAmount = reader.i32();
            item.extraAmount = reader.u16();
            item.byteA = reader.u8();
            item.byteB = reader.u8();
            item.insertItemId = reader.u16();
            item.expiry = reader.f64();
            item.fusionLevel = reader.u8();
            item.attributeAdjust = reader.u8();
            item.lookItemId = reader.u16();
            item.boxItemId = reader.u16();
            offer.items.push_back(item);
        }
        return offer;
    }

}
