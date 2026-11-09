#include "protocol/PetLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 2;
        constexpr int kEquipmentPlaces = 6;

        /// @brief Reads the pet's three skills (u8 level, i32 exp each).
        /// @param reader Reader positioned at the skills.
        /// @param pet Pet record to fill.
        void readSkills(core::ByteReader& reader, PetRecord& pet) {
            for (std::size_t i = 0; i < pet.skillLevels.size(); ++i) {
                pet.skillLevels[i] = reader.u8();
                pet.skillExp[i] = reader.i32();
            }
        }

        /// @brief Reads a 19-byte equipment record as 23/11 has it (the pet's slot comes from Item.dat).
        /// @param reader Reader positioned at the record.
        /// @return The worn item.
        ItemInstance readWorn(core::ByteReader& reader) {
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
            return item;
        }

    }

    std::vector<PetRecord> decodePetList(const core::Bytes& payload) {
        std::vector<PetRecord> pets;
        core::ByteReader reader(payload, kHeader);
        while (!reader.atEnd()) {
            PetRecord pet;
            pet.slot = reader.u8();
            pet.npcId = reader.u16();
            pet.exp = reader.i32();
            pet.level = reader.u8();
            pet.hp = reader.i32();
            pet.sp = reader.u16();
            pet.intelligence = reader.u16();
            pet.strength = reader.u16();
            pet.constitution = reader.u16();
            pet.agility = reader.u16();
            pet.wisdom = reader.u16();
            reader.skip(1);
            pet.amity = reader.u8();
            pet.tradable = reader.u8();
            // skip the u16 skill points
            reader.skip(2);
            pet.name = reader.shortText();
            readSkills(reader, pet);
            for (int i = 0; i < kEquipmentPlaces; ++i) {
                const ItemInstance item = readWorn(reader);
                if (item.itemId != 0) {
                    pet.equipment.push_back(item.itemId);
                    pet.worn.push_back(item);
                }
            }
            reader.skip(1);
            pet.training = reader.u8();
            pet.reborn = reader.u8();
            pet.potential = reader.u8();
            if (reader.failed()) {
                break;
            }
            pets.push_back(pet);
        }
        return pets;
    }

    std::vector<PetRecord> decodeStoredPets(const core::Bytes& payload) {
        std::vector<PetRecord> pets;
        core::ByteReader reader(payload, kHeader);
        while (!reader.atEnd()) {
            PetRecord pet;
            pet.slot = reader.u8();
            pet.npcId = reader.u16();
            pet.exp = reader.i32();
            pet.level = reader.u8();
            pet.hp = reader.i32();
            pet.sp = reader.u16();
            pet.intelligence = reader.u16();
            pet.strength = reader.u16();
            pet.constitution = reader.u16();
            pet.agility = reader.u16();
            pet.wisdom = reader.u16();
            reader.skip(1);
            pet.amity = reader.u8();
            pet.tradable = reader.u8();
            // skip the u16 skill points
            reader.skip(2);
            pet.name = reader.shortText();
            readSkills(reader, pet);
            for (int i = 0; i < kEquipmentPlaces; ++i) {
                const int itemId = reader.u16();
                if (itemId != 0) {
                    pet.equipment.push_back(itemId);
                }
            }
            pet.reborn = reader.u8();
            pet.potential = reader.u8();
            if (reader.failed()) {
                break;
            }
            pets.push_back(pet);
        }
        return pets;
    }

    std::optional<PetJoined> decodePetJoined(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        PetJoined joined;
        joined.ownerId = reader.i32();
        PetRecord& pet = joined.pet;
        pet.npcId = reader.i32();
        pet.tradable = reader.u8();
        pet.strength = reader.u16();
        pet.constitution = reader.u16();
        pet.intelligence = reader.u16();
        pet.wisdom = reader.u16();
        pet.agility = reader.u16();
        pet.level = reader.u8();
        pet.exp = reader.i32();
        readSkills(reader, pet);
        pet.amity = reader.u8();
        const int worn = reader.u16();
        if (worn != 0) {
            pet.equipment.push_back(worn);
        }
        pet.training = reader.u8();
        pet.reborn = reader.u8();
        pet.potential = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return joined;
    }

    std::vector<ScenePet> decodeScenePets(const core::Bytes& payload) {
        std::vector<ScenePet> pets;
        core::ByteReader reader(payload, kHeader);
        while (!reader.atEnd()) {
            ScenePet pet;
            pet.playerId = reader.i32();
            pet.npcId = reader.i32();
            pet.mounted = reader.u8() == 1;
            pet.tradable = reader.u8();
            pet.name = reader.shortText();
            pet.wornItem = reader.u16();
            if (reader.failed()) {
                break;
            }
            pets.push_back(pet);
        }
        return pets;
    }

    PetsMounted decodePetsMounted(const core::Bytes& payload) {
        PetsMounted result;
        core::ByteReader reader(payload, kHeader);
        result.playerId = reader.i32();
        while (!reader.atEnd()) {
            PetsMounted::Entry entry;
            entry.place = reader.u8();
            entry.npcId = reader.i32();
            entry.mounted = reader.u8() == 1;
            entry.tradable = reader.u8();
            entry.name = reader.shortText();
            if (reader.failed()) {
                break;
            }
            result.pets.push_back(entry);
        }
        return result;
    }

    std::vector<PetRide> decodePetRides(const core::Bytes& payload) {
        constexpr int kPlaces = 4;
        std::vector<PetRide> rides;
        core::ByteReader reader(payload, kHeader);
        while (!reader.atEnd()) {
            PetRide ride;
            ride.place = reader.u8();
            ride.playerId = reader.i32();
            ride.npcId = reader.u16();
            if (reader.failed() || ride.place < 1 || ride.place > kPlaces) {
                break;
            }
            rides.push_back(ride);
        }
        return rides;
    }

}
