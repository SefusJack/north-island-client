#include "protocol/PetTradeLists.h"

#include "core/ByteReader.h"
#include "protocol/PetInfoBlockReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kGearRest = 17; // bytes of an equipment entry after its u16 item id
        constexpr std::size_t kInfoBlock = 86; // bytes of the pet info block at the end of the offer

    }

    std::optional<PetTradeOffer> decodePetTradeOffer(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action and sub
        PetTradeOffer offer;
        offer.gold = reader.i32();
        offer.npcId = reader.u16();
        offer.level = reader.u8();
        offer.maxHp = reader.i32();
        offer.hp = reader.i32();
        offer.maxSp = reader.u16();
        offer.sp = reader.u16();
        offer.name = reader.shortText();
        offer.amity = reader.u8();
        offer.updateTimes = reader.u8();
        const int gear = reader.u8();
        for (int i = 0; i < gear; ++i) {
            offer.equipment.push_back(reader.u16());
            reader.skip(kGearRest);
        }
        if (reader.remaining() >= kInfoBlock) {
            offer.info = readPetInfoBlock(reader);
            offer.info->amity = offer.amity;
        }
        if (reader.failed()) {
            return std::nullopt;
        }
        return offer;
    }

}
