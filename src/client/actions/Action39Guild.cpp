#include "client/actions/Action39Guild.h"

#include "core/ByteReader.h"
#include "protocol/GuildLists.h"
#include "protocol/MessageId.h"

#include <iterator>

namespace client {

    namespace {

        constexpr std::size_t kBadgeBytes = 288; // bytes of one guild badge: 24 x 24 pixels, 4 bits each

    }

    void handleAction39Guild(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: { // 39/1: u8 guild creation result
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onGuildCreateResult(result);
                    return;
                }
                break;
            }
            case 2: // 39/2: the guild's info
                if (const auto info = protocol::decodeGuildInfo(payload)) {
                    context.events.onGuildInfo(*info);
                    return;
                }
                break;
            case 3: { // 39/3: i32 inviter: a guild invitation
                const std::int32_t inviter = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildInvite(inviter);
                    return;
                }
                break;
            }
            case 4: { // 39/4: i32 target, u8 answer: invitation answer
                const std::int32_t target = reader.i32();
                const int answer = reader.u8();
                if (!reader.failed()) {
                    context.events.onGuildInviteAnswer(target, answer);
                    return;
                }
                break;
            }
            case 5: { // 39/5: i32 sender, f64 send time, text: guild mail
                const std::int32_t senderId = reader.i32();
                const double sendTime = reader.f64();
                const std::string text = reader.rest();
                if (!reader.failed()) {
                    context.events.onGuildMail(senderId, sendTime, text);
                    return;
                }
                break;
            }
            case 6: { // 39/6: i32 member: a member is gone
                const std::int32_t member = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildMemberGone(member);
                    return;
                }
                break;
            }
            case 7: { // 39/7: u8 result, i32 member: leave result
                const int result = reader.u8();
                const std::int32_t member = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildLeaveResult(result, member);
                    return;
                }
                break;
            }
            case 8: { // 39/8: i32 member: a member joined
                const std::int32_t member = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildMemberJoined(member);
                    return;
                }
                break;
            }
            case 9: // 39/9: n x (i32 player, u32 guild id, pstr8 guild): players' guilds
                while (reader.remaining() > 0) {
                    const std::int32_t playerId = reader.i32();
                    const std::uint32_t guildId = reader.u32();
                    const std::string name = reader.shortText();
                    if (reader.failed()) {
                        break;
                    }
                    context.events.onGuildTitle(playerId, guildId, name);
                }
                return;
            case 30: { // 39/30: u8 version, badge pixels: the own guild badge
                const int version = reader.u8();
                const core::Bytes pixels = reader.bytes(kBadgeBytes);
                if (!reader.failed()) {
                    context.events.onOwnBadge(version, pixels);
                    return;
                }
                break;
            }
            case 31: // 39/31: badge versions list
            case 32: { // 39/32: badge versions with pixels
                std::vector<ServerEvents::BadgeRecord> records;
                while (reader.remaining() >= (id.sub == 31 ? 5u : 5u + kBadgeBytes)) { // 5: record size u32 guild + u8 version
                    ServerEvents::BadgeRecord record;
                    record.guildId = reader.u32();
                    record.version = reader.u8();
                    if (id.sub == 32) {
                        record.pixels = reader.bytes(kBadgeBytes);
                    }
                    records.push_back(std::move(record));
                }
                if (id.sub == 31) {
                    context.events.onBadgeList(records);
                } else {
                    context.events.onBadges(records);
                }
                return;
            }
            case 27: { // 39/27: u8 kind, i32 count: weapon materials in the guild bank
                const int kind = reader.u8();
                const std::int32_t count = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildWeaponCount(kind, count);
                    return;
                }
                break;
            }
            case 34: { // 39/34: u32 guild, u8 sample: a badge is reset
                const std::uint32_t guildId = reader.u32();
                const int sample = reader.u8();
                if (!reader.failed()) {
                    context.events.onBadgeReset(guildId, sample);
                    return;
                }
                break;
            }
            case 10: { // 39/10: u8 result: guild disbanded
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onGuildDisbanded(result);
                    return;
                }
                break;
            }
            case 11: // 39/11: the guild rules text
                context.events.onGuildRules(reader.rest());
                return;
            case 12: // 39/12: i32 member made vice
            case 13: { // 39/13: i32 member removed as vice
                const std::int32_t member = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildVice(member, id.sub == 12);
                    return;
                }
                break;
            }
            case 14: { // 39/14: u8 mode, i32 lord: a new lord
                const int mode = reader.u8();
                const std::int32_t lord = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildNewLord(mode, lord);
                    return;
                }
                break;
            }
            case 15: { // 39/15: u8 code: position error
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onGuildPositionError(code);
                    return;
                }
                break;
            }
            case 16: { // 39/16: f64 date: guild disband date
                const double date = reader.f64();
                if (!reader.failed()) {
                    context.events.onGuildDisbandDate(date);
                    return;
                }
                break;
            }
            case 18: // 39/18: i32: a vice offer
            case 19: { // 39/19: i32: a lord offer
                const std::int32_t lord = reader.i32();
                if (!reader.failed()) {
                    if (id.sub == 18) {
                        context.events.onGuildViceOffer(lord);
                    } else {
                        context.events.onGuildLordOffer(lord);
                    }
                    return;
                }
                break;
            }
            case 23: { // 39/23: i32 member, u8 kind, i32 value (RecvOrgMemberValue)
                const std::int32_t member = reader.i32();
                const int kind = reader.u8();
                const std::int32_t value = reader.i32();
                if (!reader.failed()) {
                    if (kind == 1) { // kind 1: the contribution
                        context.events.onGuildContribution(member, value);
                    }
                    return;
                }
                break;
            }
            case 24: { // 39/24: member rights (RecvMemberRights)
                const std::int32_t member = reader.i32();
                const int count = reader.u8();
                for (int i = 0; i < count && !reader.failed(); ++i) {
                    const int right = reader.u8();
                    const bool on = reader.u8() == 1; // 1 = the right is on
                    if (!reader.failed()) {
                        context.events.onGuildRight(member, right, on);
                    }
                }
                return;
            }
            case 25: { // 39/25: u8 post, i32 member: a post accepted
                const int post = reader.u8();
                const std::int32_t member = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildPostAccepted(post, member);
                    return;
                }
                break;
            }
            case 40: // 39/40: open the guild creation form
                context.events.onGuildCreateForm();
                return;
            case 61: { // 39/61: records of i32 member, u8 state: member online states
                std::vector<protocol::FriendState> states;
                while (reader.remaining() >= 5) { // record size: i32 + u8
                    protocol::FriendState state;
                    state.id = reader.i32();
                    state.state = reader.u8();
                    states.push_back(state);
                }
                context.events.onGuildStates(states);
                return;
            }
            case 62: { // 39/62: u8 welcome flag
                const int welcome = reader.u8();
                if (!reader.failed()) {
                    if (welcome == 1) { // 1 = show the welcome
                        context.events.onGuildWelcome();
                    }
                    return;
                }
                break;
            }
            case 20: // 39/20: the guild's mailbox is full
                context.events.onNoticeMessage("The guild's mailbox is full!!", 1200); // notice display time in ms
                return;
            case 17: // 39/17: show the guild rule help
                context.events.onGuildRuleHelp();
                return;
            case 21: // 39/21: guild resource (ignored)
            case 26: // 39/26: org value (ignored)
                return;
            case 22: { // 39/22: u8 contribution result
                const int result = reader.u8();
                if (!reader.failed()) {
                    if (result == 0) {
                        context.events.onNoticeMessage("Contribution successful", 1200); // notice display time in ms
                    } else if (result == 1) { // result 1: failed
                        context.events.onNoticeMessage("Contribution failed", 1200); // notice display time in ms
                    }
                    return;
                }
                break;
            }
            case 50: // 39/50: guild work message
            case 51: // 39/51: guild work message
            case 53: // 39/53: guild work message
            case 54: // 39/54: guild work message
            case 55: // 39/55: guild work message
            case 56: // 39/56: guild work message
            case 57: // 39/57: guild work message
            case 58: // 39/58: guild work message
            case 59: // 39/59: guild work message
                context.events.onGuildWork(payload);
                return;
            case 60: { // 39/60: one member's info record (RecvOrgMemberInfo)
                protocol::GuildMemberRecord record;
                record.id = reader.i32();
                record.name = reader.shortText();
                record.level = reader.u8();
                record.job = reader.u8();
                record.reborn = reader.u8();
                record.element = reader.u8();
                record.bodyType = reader.u8();
                record.head = reader.u8();
                record.colorCode1 = reader.i32();
                record.colorCode5 = reader.i32();
                record.nickname = reader.shortText();
                if (!reader.failed()) {
                    context.events.onGuildMemberInfo(record);
                    return;
                }
                break;
            }
            case 28: { // 39/28: u8 Ally function closed state (RecvAllyFunctionState)
                const int closed = reader.u8();
                if (!reader.failed()) {
                    context.events.onNoticeMessage(
                        closed != 0 ? "The Ally function is closed now." : "The Ally function is opened now.",
                        4000
                    ); // notice display time in ms
                    return;
                }
                break;
            }
            case 33: { // 39/33: the new guild name (RecvGuildNameChanged)
                const std::string name = reader.rest();
                if (!reader.failed()) {
                    context.events.onGuildRenamed(name);
                    return;
                }
                break;
            }
            case 63: { // 39/63: i32 member who is lord now (RecvLordChanged)
                const std::int32_t memberId = reader.i32();
                if (!reader.failed()) {
                    context.events.onGuildLordChanged(memberId);
                    return;
                }
                break;
            }
            case 29: { // 39/29: u8 what: the Trojan War forbids a guild action
                static const char* const kRefusals[] = {
                    "",
                    "Unable to leave during the Trojan War.",
                    "Unable to delete during the Trojan War.",
                    "Unable to disband during the Trojan War.",
                    "Unable to delete character during the Trojan War.",
                    "Unable to found a guild during the Trojan War.",
                    "Unable to join a guild during the Trojan War.",
                    "Unable to use the appointment function in guild during the Trojan War.",
                    "Unable to use deposal function in guild during the Trojan War.",
                    "Unable to use the demote function in guild during the Trojan War."
                };
                const std::size_t what = reader.u8();
                if (!reader.failed() && what >= 1 && what < std::size(kRefusals)) {
                    context.events.onNoticeMessage(kRefusals[what], 2000); // notice display time in ms
                    return;
                }
                break;
            }
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
