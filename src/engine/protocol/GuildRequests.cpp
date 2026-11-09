#include "protocol/GuildRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kGuild = 39; // action 39: guild

        /// @brief Builds a client 39/sub request carrying one i32 member id.
        /// @param sub Sub id.
        /// @param id Member's player id.
        /// @return The encoded request.
        core::Bytes memberRequest(std::uint8_t sub, std::int32_t id) {
            core::ByteWriter writer;
            writer.u8(kGuild).u8(sub).i32(id);
            return writer.take();
        }

        /// @brief Builds a client 39/sub request carrying an i32 player id and a u8 answer.
        /// @param sub Sub id.
        /// @param id Player id.
        /// @param answer Answer byte.
        /// @return The encoded request.
        core::Bytes answerRequest(std::uint8_t sub, std::int32_t id, int answer) {
            core::ByteWriter writer;
            writer.u8(kGuild).u8(sub).i32(id).u8(static_cast<std::uint8_t>(answer));
            return writer.take();
        }

        /// @brief Builds a client 39/sub request with no data.
        /// @param sub Sub id.
        /// @return The encoded request.
        core::Bytes plainRequest(std::uint8_t sub) {
            core::ByteWriter writer;
            writer.u8(kGuild).u8(sub);
            return writer.take();
        }

    }

    core::Bytes createGuildRequest(const std::string& name) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(1).text(name);
        return writer.take();
    }

    core::Bytes guildInviteRequest(std::int32_t playerId) { return memberRequest(2, playerId); } // 39/2: invite a player

    core::Bytes guildInviteAnswerRequest(std::int32_t inviterId, int answer) { return answerRequest(3, inviterId, answer); } // 39/3: answer an invitation

    core::Bytes leaveGuildRequest() { return plainRequest(6); } // 39/6: leave the guild

    core::Bytes kickGuildMemberRequest(std::int32_t memberId) { return memberRequest(7, memberId); } // 39/7: kick a member

    core::Bytes disbandGuildRequest() { return plainRequest(8); } // 39/8: disband the guild

    core::Bytes stopLabourRequest() { return plainRequest(50); } // 39/50: stop labour

    core::Bytes stopCraftRequest() { return plainRequest(51); } // 39/51: stop craft

    core::Bytes guildRulesRequest(const std::string& rules) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(9).text(rules); // 39/9: guild rules
        return writer.take();
    }

    core::Bytes deposeViceRequest(std::int32_t viceId) { return memberRequest(11, viceId); } // 39/11: depose a vice lord

    core::Bytes abdicateGuildRequest(std::int32_t memberId) { return memberRequest(12, memberId); } // 39/12: offer the lord's post

    core::Bytes lordOfferAnswerRequest(std::int32_t lordId, int answer) { return answerRequest(13, lordId, answer); } // 39/13: answer the lord's post offer

    core::Bytes viceOfferAnswerRequest(std::int32_t playerId, int answer) {
        return answerRequest(14, playerId, answer);
    } // 39/14: answer the vice lord's post offer

    core::Bytes guildMailRequest(const std::vector<std::int32_t>& members, const std::string& text) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(4).u8(static_cast<std::uint8_t>(members.size())); // 39/4: mail to members
        for (std::int32_t id : members) {
            writer.i32(id);
        }
        writer.text(text);
        return writer.take();
    }

    core::Bytes guildBadgeRequest(const core::Bytes& badge) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(18).bytes(badge); // 39/18: guild badge
        return writer.take();
    }

    core::Bytes guildBadgeListRequest(const std::vector<std::uint32_t>& guildIds) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(19); // 39/19: ask for badges
        for (const std::uint32_t id : guildIds) {
            writer.u32(id);
        }
        return writer.take();
    }

    core::Bytes guildRightsRequest(std::int32_t memberId, const std::array<bool, 5>& rights) { // five rights per member
        core::ByteWriter writer;
        // 39/16: member rights
        writer.u8(kGuild).u8(16).i32(memberId).u8(static_cast<std::uint8_t>(rights.size()));
        for (std::size_t i = 0; i < rights.size(); ++i) {
            writer.u8(static_cast<std::uint8_t>(i + 1)).u8(rights[i] ? 1 : 0);
        }
        return writer.take();
    }

    core::Bytes guildWeaponDepositRequest(const std::vector<std::pair<int, int>>& amounts) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(52); // 39/52: material into the weapon bank
        for (const auto& [kind, amount] : amounts) {
            writer.u8(static_cast<std::uint8_t>(kind)).u16(static_cast<std::uint16_t>(amount));
        }
        return writer.take();
    }

    core::Bytes guildWeaponWithdrawRequest(const std::vector<int>& bagSlots) {
        core::ByteWriter writer;
        writer.u8(kGuild).u8(53); // 39/53: materials out of the weapon bank
        for (const int slot : bagSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        return writer.take();
    }

}
