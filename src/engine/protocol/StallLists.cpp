#include "protocol/StallLists.h"

#include "core/ByteReader.h"
#include "protocol/PetInfoBlockReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kPriceRecord = 5; // bytes per price entry: u8 bag slot + i32 price
        constexpr std::size_t kGoodsRecord = 37; // bytes per item entry in a stall's goods
        constexpr std::size_t kPetRecord = 218; // bytes per pet entry in a stall's goods
        constexpr int kGearSlots = 6; // pet equipment places in a pet entry
        constexpr std::size_t kGearRest = 17; // bytes of an equipment entry after its u16 item id
        constexpr int kItemSale = 1;

    }

    std::optional<OwnStall> decodeOwnStall(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        OwnStall stall;
        stall.name = reader.shortText();
        stall.sign = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() >= kPriceRecord) {
            OwnStall::Price price;
            price.bagSlot = reader.u8();
            price.price = reader.i32();
            stall.prices.push_back(price);
        }
        return stall;
    }

    std::vector<StallSignRecord> decodeStallSigns(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        std::vector<StallSignRecord> signs;
        while (reader.remaining() > 0) {
            StallSignRecord record;
            record.playerId = reader.i32();
            record.name = reader.shortText();
            record.sign = reader.u8();
            if (reader.failed()) {
                break;
            }
            signs.push_back(record);
        }
        return signs;
    }

    std::optional<StallGoods> decodeStallGoods(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        StallGoods goods;
        goods.ownerId = reader.i32();
        // 0xFFFF keeps the low 16 bits of the u32 card days / points
        goods.cardDays = static_cast<int>(reader.u32() & 0xFFFF);
        goods.cardPoints = static_cast<int>(reader.u32() & 0xFFFF);
        goods.cardPrice = reader.i32();
        goods.cardKind = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() >= kGoodsRecord) {
            StallGoods::Item entry;
            entry.stallSlot = reader.u8();
            entry.item.itemId = reader.u16();
            entry.item.count = reader.u8();
            entry.item.wear = reader.u8();
            entry.price = reader.i32();
            entry.item.byteA = reader.u8();
            entry.item.byteB = reader.u8();
            entry.item.insertItemId = reader.u16();
            entry.item.expiry = reader.f64();
            entry.item.fusionLevel = reader.u8();
            entry.item.attributeAdjust = reader.u8();
            entry.item.lookItemId = reader.u16();
            entry.echoKey = reader.u32();
            entry.item.boxItemId = reader.u16();
            entry.item.usedAmount = reader.i32();
            entry.item.extraAmount = reader.u16();
            goods.items.push_back(entry);
        }
        return goods;
    }

    std::optional<PetStallGoods> decodePetStallGoods(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        PetStallGoods goods;
        goods.ownerId = reader.i32();
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() >= kPetRecord) {
            PetStallGoods::Pet pet;
            pet.stallSlot = reader.u8();
            pet.npcId = reader.u16();
            pet.price = reader.i32();
            pet.level = reader.u8();
            pet.hp = reader.i32();
            pet.maxHp = reader.i32();
            pet.maxSp = reader.u16();
            for (int gear = 0; gear < kGearSlots; ++gear) {
                const int itemId = reader.u16();
                reader.skip(kGearRest);
                if (itemId != 0) {
                    pet.equipment.push_back(itemId);
                }
            }
            pet.info = readPetInfoBlock(reader);
            goods.pets.push_back(pet);
        }
        return goods;
    }

    std::optional<StallSale> decodePetStallSale(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        StallSale sale;
        sale.kind = reader.u8();
        sale.itemId = reader.u16();
        sale.count = reader.u8();
        sale.money = reader.i32();
        if (reader.failed()) {
            return std::nullopt;
        }
        return sale;
    }

    std::optional<StallSale> decodeStallSale(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        StallSale sale;
        sale.kind = reader.u8();
        sale.itemId = reader.u16();
        sale.count = reader.u8();
        sale.money = reader.i32();
        if (sale.kind == kItemSale) {
            sale.bagSlot = reader.u8();
        }
        if (reader.failed()) {
            return std::nullopt;
        }
        return sale;
    }

}
