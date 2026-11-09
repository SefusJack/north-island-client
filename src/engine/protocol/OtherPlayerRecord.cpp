#include "protocol/OtherPlayerRecord.h"

#include "core/ByteReader.h"

namespace protocol {

    std::optional<OtherPlayerRecord> OtherPlayerRecord::decode(const core::Bytes& payload) {
        core::ByteReader reader(payload);
        const std::uint8_t action = reader.u8();
        if (action != 3 && action != 4) { // server action 3: entering, 4: already there
            return std::nullopt;
        }
        OtherPlayerRecord record;
        record.playerId = reader.i32();
        record.appearance.bodyType = reader.u8();
        record.element = reader.u8();
        record.level = reader.u8();
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
        record.onlineStatus = reader.u8();
        record.arenaVictory = reader.u8();
        record.reborn = reader.u8();
        record.job = reader.u8();
        record.name = reader.shortText();
        record.nickname = reader.shortText();
        if (reader.failed()) {
            return std::nullopt;
        }
        const int heartNumber = reader.u8();
        if (!reader.failed()) {
            record.heartNumber = heartNumber;
        }
        return record;
    }

}
