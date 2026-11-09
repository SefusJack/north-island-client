#include "protocol/GuildLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr int kFirstVice = 2;
        constexpr int kLastVice = 5;
        constexpr int kRights = 5;
        constexpr int kUnreadFlags = 5;

        /// @brief Reads one guild member record of server 39/2.
        /// @param reader Reader positioned at the record.
        /// @return The member.
        GuildMemberRecord readMember(core::ByteReader& reader) {
            GuildMemberRecord member;
            member.id = reader.i32();
            member.name = reader.shortText();
            member.level = reader.u8();
            member.job = reader.u8();
            member.reborn = reader.u8();
            member.element = reader.u8();
            member.bodyType = reader.u8();
            member.head = reader.u8();
            member.colorCode1 = reader.i32();
            member.colorCode5 = reader.i32();
            member.nickname = reader.shortText();
            member.contribution = reader.i32();
            for (int i = 0; i < kRights; ++i) {
                member.rights[static_cast<std::size_t>(i)] = reader.u8() == 1;
            }
            reader.skip(kUnreadFlags);
            return member;
        }

    }

    std::optional<GuildInfo> decodeGuildInfo(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        GuildInfo info;
        info.name = reader.shortText();
        info.viceCount = reader.u8();
        const int slots = reader.u8();
        for (int slot = 1; slot <= slots && !reader.failed(); ++slot) {
            if (slot >= kFirstVice && slot <= kLastVice && slot - 1 > info.viceCount) {
                continue;
            }
            info.members.push_back(readMember(reader));
        }
        info.rules = reader.shortText();
        info.disbandDate = reader.f64();
        info.founded = reader.f64();
        if (reader.failed()) {
            return std::nullopt;
        }
        return info;
    }

}
