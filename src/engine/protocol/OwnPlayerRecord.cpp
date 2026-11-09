#include "protocol/OwnPlayerRecord.h"

#include "core/ByteReader.h"

namespace protocol {

    std::optional<OwnPlayerRecord> OwnPlayerRecord::decode(const core::Bytes& payload) {
        core::ByteReader reader(payload);
        if (reader.u8() != 3) { // server action 3: enter the world
            return std::nullopt;
        }
        OwnPlayerRecord record;
        record.playerId = reader.i32();
        record.appearance.bodyType = reader.u8();
        record.mapId = reader.u16();
        record.x = reader.u16();
        record.y = reader.u16();
        reader.skip(1);
        record.appearance.head = reader.u8();
        record.appearance.bodySprite = reader.u8();
        record.appearance.colorCode1 = reader.i32();
        record.appearance.colorCode2 = reader.i32();
        record.appearance.readEquipment(reader);
        record.pvpWins = reader.u16();
        record.pvpRank = reader.u8();
        record.arenaVictory = reader.u8();
        record.name = reader.shortText();
        record.nickname = reader.shortText();
        if (reader.failed()) {
            return std::nullopt;
        }
        const int profileChoice = reader.u8();
        const int birthYear = reader.u8();
        const int birthMonth = reader.u8();
        const int birthDay = reader.u8();
        if (!reader.failed()) {
            record.profileChoice = profileChoice;
            record.birthYear = birthYear;
            record.birthMonth = birthMonth;
            record.birthDay = birthDay;
        }
        return record;
    }

}
