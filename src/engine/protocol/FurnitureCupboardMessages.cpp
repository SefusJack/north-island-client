#include "protocol/FurnitureCupboardMessages.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture
        constexpr std::uint8_t kPut = 41; // 62/41: put an item in the cupboard
        constexpr std::uint8_t kTake = 42; // 62/42: take an item from the cupboard
        constexpr std::size_t kMessageHead = 2; // bytes before the body: action, sub
        constexpr std::size_t kRecordSize = 50; // bytes of the stored item record

    }

    std::optional<FurnitureCupboardItem> decodeFurnitureCupboardItem(const core::Bytes& payload) {
        core::ByteReader reader(payload, kMessageHead);
        FurnitureCupboardItem stored;
        stored.furnIndex = reader.u16();
        stored.place = reader.u8();
        stored.id = reader.u8();
        if (reader.failed() || reader.remaining() < kRecordSize) {
            return std::nullopt;
        }
        ItemInstance& item = stored.item;
        reader.skip(1);
        item.itemId = reader.u16();
        item.count = reader.u8();
        item.wear = reader.u8();
        reader.skip(8); // record bytes between wear and the used amount at 13
        item.usedAmount = reader.i32();
        reader.skip(4); // record bytes between the used amount and the box at 21
        item.boxItemId = reader.u16();
        reader.skip(1); // record byte between the box and the extra amount at 24
        item.extraAmount = reader.u16();
        item.insertItemId = reader.u16();
        item.expiry = reader.f64();
        item.fusionLevel = reader.u8();
        item.attributeAdjust = reader.u8();
        item.lookItemId = reader.u16();
        return stored;
    }

    core::Bytes putInCupboardRequest(int piece, int bagSlot, int place) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(kPut).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(bagSlot)).u8(static_cast<std::uint8_t>(place));
        return writer.take();
    }

    core::Bytes takeFromCupboardRequest(int piece, int place) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(kTake).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(place));
        return writer.take();
    }

}
