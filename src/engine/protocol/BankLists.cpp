#include "protocol/BankLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 2;
        constexpr std::size_t kListRecord = 29; // bytes per list record: u8 slot + 28-byte item

        /// @brief Reads one 28-byte bank item record (u16 item, u8 count, u8 wear, u8, u8, u16 crystal, f64 expiry, u8 fusion, u8 attribute, u16 look, u16 box, i32 used, u16 extra).
        /// @param reader Reader positioned at the record.
        /// @return The item.
        ItemInstance readBankItem(core::ByteReader& reader) {
            ItemInstance item;
            item.itemId = reader.u16();
            item.count = reader.u8();
            item.wear = reader.u8();
            item.byteA = reader.u8();
            item.byteB = reader.u8();
            item.insertItemId = reader.u16();
            item.expiry = reader.f64();
            item.fusionLevel = reader.u8();
            item.attributeAdjust = reader.u8();
            item.lookItemId = reader.u16();
            item.boxItemId = reader.u16();
            item.usedAmount = reader.i32();
            item.extraAmount = reader.u16();
            return item;
        }

    }

    std::vector<BagItemRecord> decodeBankItems(const core::Bytes& payload) {
        std::vector<BagItemRecord> records;
        if (payload.size() < kHeader) {
            return records;
        }
        const std::size_t count = (payload.size() - kHeader) / kListRecord;
        core::ByteReader reader(payload, kHeader);
        for (std::size_t i = 0; i < count; ++i) {
            BagItemRecord record;
            record.slot = reader.u8();
            record.item = readBankItem(reader);
            records.push_back(record);
        }
        return records;
    }

    std::optional<ItemInstance> decodeBankItem(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        const ItemInstance item = readBankItem(reader);
        if (reader.failed()) {
            return std::nullopt;
        }
        return item;
    }

    std::optional<BagItemRecord> decodeBankItemPut(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        BagItemRecord record;
        record.slot = reader.u8();
        record.item = readBankItem(reader);
        if (reader.failed()) {
            return std::nullopt;
        }
        return record;
    }

    std::optional<BagItemRecord> decodeBankItemAtSlot(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        BagItemRecord record;
        record.item = readBankItem(reader);
        record.slot = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return record;
    }

}
