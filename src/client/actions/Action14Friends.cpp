#include "client/actions/Action14Friends.h"

#include "core/ByteReader.h"
#include "protocol/FriendLists.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction14Friends(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        switch (id.sub) {
            case 1: { // 14/1: i32 sender, f64 time, text: mail
                const std::int32_t senderId = reader.i32();
                const double sendTime = reader.f64();
                const std::string text = reader.rest();
                if (!reader.failed()) {
                    context.events.onMail(senderId, sendTime, text);
                    return;
                }
                break;
            }
            case 6: // 14/6: the receiver's mail box is full
                context.events.onMailboxFull();
                return;
            case 16: // 14/16: fur mail, a second u8 picks the arm
                switch (reader.u8()) {
                    case 1: { // 14/16/1: u8 code, result of a fur mail
                        const int code = reader.u8();
                        if (!reader.failed()) {
                            context.events.onFurMailResult(code);
                            return;
                        }
                        break;
                    }
                    case 2: { // 14/16/2: i32 sender, name, f64 time, text: a fur mail
                        const std::int32_t senderId = reader.i32();
                        const std::string name = reader.shortText();
                        const double sendTime = reader.f64();
                        const std::string text = reader.rest();
                        if (!reader.failed()) {
                            context.events.onFurMail(senderId, name, sendTime, text);
                            return;
                        }
                        break;
                    }
                    case 3: // 14/16/3: the fur mail box
                        context.events.onFurMailBox();
                        return;
                    default: // unknown fur mail arm: unhandled
                        break;
                }
                break;
            case 10: // 14/10: too much mail sent
                context.events.onSentMailFull();
                return;
            case 2: { // 14/2: i32 player asks to be friends
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onFriendRequest(playerId);
                    return;
                }
                break;
            }
            case 3: { // 14/3: i32 player, u8 answer to the own friend request
                const std::int32_t playerId = reader.i32();
                const int answer = reader.u8();
                if (!reader.failed()) {
                    context.events.onFriendAnswer(playerId, answer);
                    return;
                }
                break;
            }
            case 4: { // 14/4: i32 friend left the list
                const std::int32_t friendId = reader.i32();
                if (!reader.failed()) {
                    context.events.onFriendRemoved(friendId);
                    return;
                }
                break;
            }
            case 5: // 14/5: the friend list (FriendLists)
                context.events.onFriendList(protocol::decodeFriendList(payload));
                return;
            case 7: { // 14/7: i32 player, pstr8 guild name
                const std::int32_t playerId = reader.i32();
                const std::string guild = reader.shortText();
                if (!reader.failed()) {
                    context.events.onGuildName(playerId, guild);
                    return;
                }
                break;
            }
            case 8: // 14/8: a friend's record
            case 11: { // 14/11: a friend's record (added when new)
                // sub 8 is the info record, sub 11 the entry record
                const auto record = id.sub == 8 ? protocol::decodeFriendInfo(payload) : protocol::decodeFriendEntry(payload);
                if (record) {
                    if (id.sub == 8) {
                        context.events.onFriendInfo(*record);
                    } else {
                        context.events.onFriendEntry(*record);
                    }
                    return;
                }
                break;
            }
            case 9: // 14/9: online states
                context.events.onFriendStates(protocol::decodeFriendStates(payload));
                return;
            case 12: // 14/12: the asked player's list is full
                context.events.onFriendListFull();
                return;
            case 13: { // 14/13: u8 result (0 failed, 1 added, 2 removed, 3 the list) and black list records
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onBlacklist(result, protocol::decodeBlacklist(payload));
                    return;
                }
                break;
            }
            case 14: { // 14/14: u8 code, the player asked has you on their black list
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onBlockedBy(code);
                    return;
                }
                break;
            }
            case 15: // 14/15: the target is playing games (notice)
                context.events.onNoticeMessage("Target is playing games, cannot make frends", 1500); // notice duration in ms
                return;
            default: // any other sub id: unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
