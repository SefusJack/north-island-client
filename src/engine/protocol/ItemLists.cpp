#include "protocol/ItemLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 2;
        constexpr std::size_t kBagRecord = 29; // bytes per bag record: u8 slot + 28-byte item
        constexpr std::size_t kEquipmentRecord = 19; // bytes per equipment record

        /// @brief Reads one 29-byte bag record (slot and item fields).
        /// @param reader Reader positioned at the record.
        /// @return The record.
        BagItemRecord readBagItem(core::ByteReader& reader) {
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
            return record;
        }

    }

    std::vector<BagItemRecord> decodeBagItems(const core::Bytes& payload) {
        std::vector<BagItemRecord> records;
        if (payload.size() < kHeader) {
            return records;
        }
        const std::size_t count = (payload.size() - kHeader) / kBagRecord;
        core::ByteReader reader(payload, kHeader);
        for (std::size_t i = 0; i < count; ++i) {
            records.push_back(readBagItem(reader));
        }
        return records;
    }

    std::optional<BagItemRecord> decodeBagItem(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        BagItemRecord record = readBagItem(reader);
        if (reader.failed()) {
            return std::nullopt;
        }
        return record;
    }

    std::vector<ItemInstance> decodeEquipment(const core::Bytes& payload) {
        std::vector<ItemInstance> items;
        if (payload.size() < kHeader) {
            return items;
        }
        const std::size_t count = (payload.size() - kHeader) / kEquipmentRecord;
        core::ByteReader reader(payload, kHeader);
        for (std::size_t i = 0; i < count; ++i) {
            ItemInstance item;
            item.itemId = reader.u16();
            item.count = 1;
            item.wear = reader.u8();
            item.byteA = reader.u8();
            item.byteB = reader.u8();
            item.insertItemId = reader.u16();
            item.expiry = reader.f64();
            item.fusionLevel = reader.u8();
            item.attributeAdjust = reader.u8();
            item.lookItemId = reader.u16();
            items.push_back(item);
        }
        return items;
    }

}
