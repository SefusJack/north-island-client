#include "protocol/CharacterList.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace protocol {

    std::optional<CharacterList> CharacterList::decode(const core::Bytes& payload) {
        core::ByteReader reader(payload);
        if (reader.u8() != kActionAccount || reader.u8() != 1) {
            return std::nullopt;
        }
        CharacterList list;
        while (!reader.atEnd()) {
            CharacterSummary c;
            c.slot = reader.u8();
            c.name = reader.shortText();
            c.level = reader.u8();
            c.element = reader.u8();
            c.maxHp = reader.i32();
            c.hp = reader.i32();
            c.maxSp = reader.i32();
            c.sp = reader.i32();
            c.exp = reader.i32();
            c.gold = reader.i32();
            c.bodyType = reader.u8();
            c.appearance = reader.u8();
            c.head = reader.u8();
            c.spriteId = reader.u8();
            c.colorPattern1 = reader.i32();
            c.colorPattern2 = reader.i32();
            c.reborn = reader.u8();
            c.job = reader.u8();
            for (std::uint16_t& item : c.equipment) {
                item = reader.u16();
            }
            if (reader.failed()) {
                return std::nullopt;
            }
            list.characters.push_back(c);
        }
        return list;
    }

}
