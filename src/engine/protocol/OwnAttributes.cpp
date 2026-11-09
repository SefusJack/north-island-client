#include "protocol/OwnAttributes.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kBonuses = 6 * 4;

    }

    std::optional<OwnAttributes> OwnAttributes::decode(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action and sub
        OwnAttributes attributes;
        attributes.element = reader.u8();
        attributes.hp = reader.i32();
        attributes.sp = reader.u16();
        for (int& stat : attributes.stats) {
            stat = reader.u16();
        }
        attributes.level = reader.u8();
        attributes.exp = reader.i32();
        attributes.skillPoints = reader.u16();
        attributes.statPoints = reader.u16();
        attributes.maxHp = reader.i32();
        attributes.maxSp = reader.u16();
        reader.skip(kBonuses);
        attributes.guildId = reader.u32();
        const int skillCount = reader.u16();
        for (int i = 0; i < skillCount; ++i) {
            LearnedSkill skill;
            skill.key = reader.u16();
            skill.level = reader.u8();
            skill.exp = reader.i32();
            attributes.skills.push_back(skill);
        }
        reader.skip(2 + 2); // two unread u16 fields
        attributes.reborn = reader.u8();
        attributes.job = reader.u8();
        attributes.potential = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return attributes;
    }

}
