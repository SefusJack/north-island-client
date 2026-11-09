#include "protocol/FurnitureStallItem.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kMessageHead = 2; // bytes before the body: action, sub
        constexpr std::size_t kRecordSize = 50; // bytes of the stalled item record

    }

    std::optional<FurnitureStallItem> decodeFurnitureStallItem(const core::Bytes& payload) {
        core::ByteReader reader(payload, kMessageHead);
        FurnitureStallItem stalled;
        stalled.ownerId = reader.i32();
        stalled.stallSlot = reader.u16();
        stalled.furnIndex = reader.u16();
        stalled.posInFurn = reader.u8();
        stalled.gridCell = reader.u8();
        stalled.count = reader.u8();
        stalled.price = reader.i32();
        if (reader.failed() || reader.remaining() < kRecordSize) {
            return std::nullopt;
        }
        core::ByteReader record(payload, payload.size() - reader.remaining());
        // the skip(n) calls jump over record bytes the client does not keep, up to the next field
        record.skip(1);
        stalled.item.itemId = record.u16();
        stalled.echoItem = stalled.item.itemId;
        record.skip(1);
        stalled.item.wear = record.u8();
        record.skip(1);
        stalled.echoPrice = record.i32();
        record.skip(3);
        stalled.item.usedAmount = record.i32();
        record.skip(4);
        stalled.item.boxItemId = record.u16();
        record.skip(3);
        stalled.item.insertItemId = record.u16();
        stalled.item.expiry = record.f64();
        stalled.item.fusionLevel = record.u8();
        stalled.item.attributeAdjust = record.u8();
        stalled.item.lookItemId = record.u16();
        stalled.item.count = stalled.count;
        return stalled;
    }

}
