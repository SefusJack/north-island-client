#include "protocol/FurnitureBedMessages.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

#include <algorithm>

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture
        constexpr std::uint8_t kPut = 43; // 62/43: put a pet in the bed
        constexpr std::uint8_t kTake = 44; // 62/44: take a pet out of the bed
        constexpr std::size_t kMessageHead = 2; // bytes before the body: action, sub
        constexpr std::size_t kRecordSize = 339; // bytes of the stored pet record
        constexpr int kNameBytes = 20; // longest pet name, bytes
        constexpr int kSkills = 3; // pet skill slots in the record
        constexpr int kEquipmentPlaces = 6; // pet equipment places in the record
        constexpr std::size_t kEquipmentSize = 34; // bytes per equipment entry in the record
        constexpr std::size_t kFirstEquipment = 66 + kEquipmentSize; // record offset of the first kept equipment entry (66 + one entry)

    }

    std::optional<FurnitureBedPet> decodeFurnitureBedPet(const core::Bytes& payload) {
        core::ByteReader head(payload, kMessageHead);
        FurnitureBedPet stored;
        stored.furnIndex = head.u16();
        stored.bed = head.u8();
        stored.id = head.u8();
        if (head.failed() || head.remaining() < kRecordSize) {
            return std::nullopt;
        }
        const std::size_t start = head.position();
        core::ByteReader reader(payload, start);
        PetRecord& pet = stored.pet;
        pet.npcId = reader.u16();
        const int length = reader.u8();
        pet.name = reader.text(static_cast<std::size_t>(std::min(length, kNameBytes)));
        reader.seek(start + 23); // record offset after u16 npc and the 20-byte name with its length byte
        pet.hp = reader.u16();
        pet.sp = reader.u16();
        reader.skip(2); // STR (u16), not kept
        pet.constitution = reader.u16();
        reader.skip(2); // INT (u16), not kept
        pet.wisdom = reader.u16();
        reader.skip(2); // AGI (u16), not kept
        pet.level = reader.u8();
        pet.exp = reader.i32();
        reader.skip(3); // u16 skill points and u8 inn, not kept
        pet.amity = reader.u8();
        for (int i = 0; i < kSkills; ++i) {
            pet.skillLevels[static_cast<std::size_t>(i)] = reader.u8();
            reader.skip(4); // the skill's i32 exp, not kept
        }
        for (int i = 0; i < kEquipmentPlaces; ++i) {
            reader.seek(start + kFirstEquipment + static_cast<std::size_t>(i) * kEquipmentSize);
            if (const int itemId = reader.u16(); itemId != 0) {
                pet.equipment.push_back(itemId);
            }
        }
        reader.seek(start + 304); // record offset of the tradable flag
        pet.tradable = reader.u8();
        reader.seek(start + 311); // record offset of the reborn byte
        pet.reborn = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return stored;
    }

    core::Bytes putPetInBedRequest(int piece, int petPlace) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(kPut).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(petPlace));
        return writer.take();
    }

    core::Bytes takePetFromBedRequest(int piece, int bed) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(kTake).u16(static_cast<std::uint16_t>(piece)).u8(static_cast<std::uint8_t>(bed));
        return writer.take();
    }

}
