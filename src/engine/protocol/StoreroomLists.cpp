#include "protocol/StoreroomLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 2;
        constexpr std::size_t kBagRecord = 29; // bytes per record: u8 slot + 28-byte item
        constexpr std::size_t kPieceBytes = 2; // bytes of the u16 furniture piece after the header

    }

    std::optional<StoreroomList> decodeStoreroomList(const core::Bytes& payload) {
        if (payload.size() < kHeader + kPieceBytes) {
            return std::nullopt;
        }
        StoreroomList list;
        const std::size_t count = (payload.size() - kHeader - kPieceBytes) / kBagRecord;
        core::ByteReader reader(payload, kHeader);
        for (std::size_t i = 0; i < count; ++i) {
            BagItemRecord record;
            record.slot = reader.u8();
            ItemInstance& item = record.item;
            item.itemId = reader.u16();
            item.count = reader.u8();
            item.wear = reader.u8();
            item.byteA = reader.u8();
            item.byteB = reader.u8();
            item.usedAmount = reader.i32();
            item.extraAmount = reader.u16();
            item.insertItemId = reader.u16();
            item.expiry = reader.f64();
            item.fusionLevel = reader.u8();
            item.attributeAdjust = reader.u8();
            item.lookItemId = reader.u16();
            item.boxItemId = reader.u16();
            list.records.push_back(record);
        }
        list.piece = reader.u16();
        if (reader.failed()) {
            return std::nullopt;
        }
        return list;
    }

    std::optional<StoreroomAdd> decodeStoreroomAdd(const core::Bytes& payload, bool namedCell) {
        core::ByteReader reader(payload, kHeader);
        StoreroomAdd add;
        ItemInstance& item = add.record.item;
        item.itemId = reader.u16();
        item.count = reader.u8();
        item.wear = reader.u8();
        item.byteA = reader.u8();
        item.byteB = reader.u8();
        item.usedAmount = reader.i32();
        item.extraAmount = reader.u16();
        add.record.slot = namedCell ? reader.u8() : 0;
        item.insertItemId = reader.u16();
        item.expiry = reader.f64();
        item.fusionLevel = reader.u8();
        item.attributeAdjust = reader.u8();
        item.lookItemId = reader.u16();
        item.boxItemId = reader.u16();
        add.showMessage = reader.u8() == 1;
        if (reader.failed()) {
            return std::nullopt;
        }
        return add;
    }

    std::optional<StoreroomRemove> decodeStoreroomRemoveCell(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        StoreroomRemove remove;
        remove.cell = reader.u8();
        remove.count = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return remove;
    }

    std::optional<StoreroomRemove> decodeStoreroomRemoveItem(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        StoreroomRemove remove;
        remove.itemId = reader.u16();
        remove.count = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return remove;
    }

}
