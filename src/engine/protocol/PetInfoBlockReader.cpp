#include "protocol/PetInfoBlockReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kBeforeHp = 21; // bytes skipped between the u16 npc id and the i32 HP
        constexpr std::size_t kBeforeAmity = 7; // bytes skipped between the level and the amity
        constexpr std::size_t kAfterAmity = 38; // bytes skipped after the amity, to the end of the info block

    }

    PetInfoBlock readPetInfoBlock(core::ByteReader& reader) {
        PetInfoBlock block;
        block.npcId = reader.u16();
        reader.skip(kBeforeHp);
        block.hp = reader.i32();
        block.sp = reader.u16();
        block.strength = reader.u16();
        block.constitution = reader.u16();
        block.intelligence = reader.u16();
        block.wisdom = reader.u16();
        block.agility = reader.u16();
        block.level = reader.u8();
        reader.skip(kBeforeAmity);
        block.amity = reader.u8();
        reader.skip(kAfterAmity);
        return block;
    }

}
