#include "client/actions/Action05Status.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/OwnAttributes.h"

#include <array>
#include <optional>

namespace client {

    namespace {

        /// @brief Reads the optional trailing value of a short status packet.
        /// @details Original (HandleDoubleExpPacket: Len > 2, Len > 3): nothing, a byte, or an i32 when four bytes follow.
        /// @param reader Reader positioned after the fixed fields.
        /// @return The value, or std::nullopt when the packet ends here.
        std::optional<int> optionalValue(core::ByteReader& reader) {
            if (reader.atEnd()) {
                return std::nullopt;
            }
            return reader.remaining() >= 4 ? reader.i32() : reader.u8(); // 4 bytes left = an i32
        }

    }

    void handleAction05Status(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 12: { // 5/12: i32 player, u8 role (RecvRoleChange)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                const int role = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onRoleChange(playerId, role);
                break;
            }
            case 0: { // 5/0: i32 player, u16 ids of the items worn (RecvPlayerEquipList)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                std::vector<int> items;
                while (reader.remaining() >= 2) { // 2 bytes = one u16 item id
                    items.push_back(reader.u16());
                }
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPlayerEquipList(playerId, items);
                break;
            }
            case 3: // 5/3: all of the own player's attributes (OwnAttributes)
                if (const auto attributes = protocol::OwnAttributes::decode(payload)) {
                    context.events.onOwnAttributes(*attributes);
                }
                break;
            case 4: // 5/4: the player is in the world (players are drawn and may walk)
                context.events.onWorldReady();
                break;
            case 22: { // 5/22: u8 grade, praise message (ShowPraiseMsg)
                core::ByteReader reader(payload, 2);
                const int grade = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPraise(grade);
                break;
            }
            case 10: { // 5/10: i32 player, level up (RecvLevelUp)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPlayerLevelUp(playerId);
                break;
            }
            case 11: { // 5/11: three u16 recent battle skills
                core::ByteReader reader(payload, 2);
                std::array<int, 3> skills{}; // 3 recent battle skills
                for (int& skill : skills) {
                    skill = reader.u16();
                }
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onRecentBattleSkills(skills);
                break;
            }
            case 21: { // 5/21: u8 code, the return spot was recorded
                core::ByteReader reader(payload, 2);
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onReturnSpotRecorded(code);
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            case 13: // 5/13: book case slot (HandleBookCaseSlotPacket)
            case 24: { // 5/24: book desk slot (HandleBookDeskSlotPacket)
                core::ByteReader reader(payload, 2);
                const int place = reader.u8();
                const int itemId = reader.u16();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onBookShelfSlot(id.sub == 24, place, itemId); // 5/24 is the desk, 5/13 the case
                break;
            }
            case 1: // 5/1: equipment taken off (RecvPlayerEquipChange)
            case 2: { // 5/2: equipment put on (RecvPlayerEquipChange)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                const int itemId = reader.u16();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPlayerEquipChanged(playerId, itemId, id.sub == 2); // 5/2 puts the item on
                break;
            }
            case 8: { // 5/8: i32 player, u8 online status (RecvPlayerOnlineStatus)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                const int status = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPlayerOnlineStatus(playerId, status);
                break;
            }
            case 5: { // 5/5: records of i32 player, u16 NPC (RecvHumanTransform)
                core::ByteReader reader(payload, 2);
                while (reader.remaining() >= 6) { // record size: i32 + u16
                    const std::int32_t playerId = reader.i32();
                    const int npcId = reader.u16();
                    context.events.onHumanTransform(playerId, npcId);
                }
                break;
            }
            case 23: { // 5/23: i32 player, u8 job (RecvPlayerJob)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                const int job = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPlayerJob(playerId, job);
                break;
            }
            case 14: // 5/14: double experience (HandleDoubleExpPacket)
            case 16: { // 5/16: spa buns (HandleSpaBunPacket)
                core::ByteReader reader(payload, 2);
                const int kind = reader.u8();
                const std::optional<int> value = optionalValue(reader);
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                id.sub == 14 ? context.events.onDoubleExp(kind, value) : context.events.onSpaBuns(kind, value); // 5/14 double exp, else 5/16 spa buns
                break;
            }
            case 15: { // 5/15: lucky grass (HandleLuckyGrassPacket)
                core::ByteReader reader(payload, 2);
                const int kind = reader.u8();
                const int minutes = kind == 1 || kind == 4 ? reader.i32() : 0; // kinds 1 and 4 carry i32 minutes
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onLuckyGrass(kind, minutes);
                break;
            }
            case 17: { // 5/17: exp potion (HandleExpPotionPacket)
                core::ByteReader reader(payload, 2);
                const int kind = reader.u8();
                const int first = reader.u8();
                int minutes = 0;
                int rate = 0;
                if (kind == 1) { // kind 1: u8 rate follows
                    rate = reader.u8();
                } else if (kind == 3) { // kind 3: i32 minutes, u8 rate follow
                    minutes = reader.i32();
                    rate = reader.u8();
                }
                if (reader.failed() || kind < 1 || kind > 4) { // valid kinds are 1..4
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onExpPotion(kind, first, minutes, rate);
                break;
            }
            case 19: { // 5/19: records of i32 player, u8 riding flag (RecvPlayerFlagList)
                core::ByteReader reader(payload, 2);
                while (reader.remaining() >= 5) { // record size: i32 + u8
                    const std::int32_t playerId = reader.i32();
                    context.events.onPlayerRidingFlag(playerId, reader.u8() != 0);
                }
                break;
            }
            case 20: { // 5/20: records of i32 player, u8 flag (RecvTrojanWarTargets)
                core::ByteReader reader(payload, 2);
                while (reader.remaining() >= 5) { // record size: i32 + u8
                    const std::int32_t playerId = reader.i32();
                    context.events.onTrojanWarFlag(playerId, reader.u8() != 0);
                }
                break;
            }
            case 18: { // 5/18: i32 player, u8 level (RecvPlayerLevel)
                core::ByteReader reader(payload, 2);
                const std::int32_t player = reader.i32();
                const int level = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onPlayerLevel(player, level);
                break;
            }
            case 25: { // 5/25: u8 code, recording coordinates failed (ShowRecordCoordFailMsg)
                core::ByteReader reader(payload, 2);
                const int code = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                if (code == 5) { // code 5: in the carnie
                    context.events.onNoticeMessage("Cannot record coordinates within the carnie.", 4000); // notice duration in ms
                }
                break;
            }
            case 26: // 5/26: i32 player, carnie team refused
            case 27: { // 5/27: i32 player, carnie team refused
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    break;
                }
                context.events.onCarnieTeamRefused(playerId);
                break;
            }
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
