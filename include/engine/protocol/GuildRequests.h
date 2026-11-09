#ifndef NORTH_ISLAND_PROTOCOL_GUILD_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_GUILD_REQUESTS_H

#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace protocol {

    /// @brief Builds client 39/1: found a guild with that name.
    /// @param name Guild name text.
    /// @return The encoded request.
    core::Bytes createGuildRequest(const std::string& name);

    /// @brief Builds client 39/2: invite a player.
    /// @param playerId Player id (i32).
    /// @return The encoded request.
    core::Bytes guildInviteRequest(std::int32_t playerId);

    /// @brief Builds client 39/3: answer an invitation.
    /// @param inviterId Inviter's player id (i32).
    /// @param answer 1 yes, 2 no, 3 no answer.
    /// @return The encoded request.
    core::Bytes guildInviteAnswerRequest(std::int32_t inviterId, int answer);

    /// @brief Builds client 39/6: leave the guild.
    /// @return The encoded request.
    core::Bytes leaveGuildRequest();

    /// @brief Builds client 39/7: send a member away.
    /// @param memberId Member's player id (i32).
    /// @return The encoded request.
    core::Bytes kickGuildMemberRequest(std::int32_t memberId);

    /// @brief Builds client 39/8: disband the guild.
    /// @return The encoded request.
    core::Bytes disbandGuildRequest();

    /// @brief Builds client 39/50: the player walks off a labour job (original StopLabour).
    /// @return The encoded request.
    core::Bytes stopLabourRequest();

    /// @brief Builds client 39/51: the player walks off a craft job (original StopCraft).
    /// @return The encoded request.
    core::Bytes stopCraftRequest();

    /// @brief Builds client 39/9: set the guild rules.
    /// @param rules Rules text.
    /// @return The encoded request.
    core::Bytes guildRulesRequest(const std::string& rules);

    /// @brief Builds client 39/11: take a vice lord's post away.
    /// @param viceId Vice lord's player id (i32).
    /// @return The encoded request.
    core::Bytes deposeViceRequest(std::int32_t viceId);

    /// @brief Builds client 39/12: offer the lord's post to a member.
    /// @param memberId Member's player id (i32).
    /// @return The encoded request.
    core::Bytes abdicateGuildRequest(std::int32_t memberId);

    /// @brief Builds client 39/13: answer the offer of the lord's post.
    /// @param lordId Lord's player id (i32).
    /// @param answer Answer (u8).
    /// @return The encoded request.
    core::Bytes lordOfferAnswerRequest(std::int32_t lordId, int answer);

    /// @brief Builds client 39/14: answer the offer of a vice lord's post (or, from the lord, appoint at once).
    /// @param playerId Player id (i32).
    /// @param answer Answer (u8).
    /// @return The encoded request.
    core::Bytes viceOfferAnswerRequest(std::int32_t playerId, int answer);

    /// @brief Builds client 39/4: mail to guild members (u8 count, count x i32 member, text).
    /// @param members Member player ids.
    /// @param text Mail text.
    /// @return The encoded request.
    core::Bytes guildMailRequest(const std::vector<std::int32_t>& members, const std::string& text);

    /// @brief Builds client 39/18: the guild badge (288 bytes, the 24 x 24 four-bit pixels of an .ens file).
    /// @param badge The badge pixel bytes.
    /// @return The encoded request.
    core::Bytes guildBadgeRequest(const core::Bytes& badge);

    /// @brief Builds client 39/19: ask for the badges the client does not have (at most ten).
    /// @param guildIds Guild ids (u32 each).
    /// @return The encoded request.
    core::Bytes guildBadgeListRequest(const std::vector<std::uint32_t>& guildIds);

    /// @brief Builds client 39/16: the rights a member has (u8 5, then 5 x (u8 right, u8 on)).
    /// @param memberId Member's player id (i32).
    /// @param rights The five rights.
    /// @return The encoded request.
    core::Bytes guildRightsRequest(std::int32_t memberId, const std::array<bool, 5>& rights);

    /// @brief Builds client 39/52: material put into the guild's weapon bank (original BankGoldLeftClick).
    /// @param amounts Pairs of kind (u8, 1..7) and amount (u16).
    /// @return The encoded request.
    core::Bytes guildWeaponDepositRequest(const std::vector<std::pair<int, int>>& amounts);

    /// @brief Builds client 39/53: the chosen weapon materials taken out of the bank (original BankGoldRightClick).
    /// @param bagSlots Bag places (u8 each).
    /// @return The encoded request.
    core::Bytes guildWeaponWithdrawRequest(const std::vector<int>& bagSlots);

}

#endif
