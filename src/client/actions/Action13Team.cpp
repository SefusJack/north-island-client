#include "client/actions/Action13Team.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/TeamLists.h"

#include <vector>

namespace client {

    void handleAction13Team(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        switch (id.sub) {
            case 1: { // 13/1: i32 requester asks to join
                const std::int32_t requesterId = reader.i32();
                if (!reader.failed()) {
                    context.events.onTeamJoinRequest(requesterId);
                    return;
                }
                break;
            }
            case 2: // 13/2: u8 1, the target is busy
                if (reader.u8() == 1) {
                    context.events.onTeamBusy();
                }
                return;
            case 3: // 13/3: u8 answer, i32 leader, the answer to the own request
            case 10: { // 13/10: u8 answer, i32 invitee, the answer to an invitation
                const int answer = reader.u8();
                const std::int32_t otherId = reader.i32();
                if (!reader.failed()) {
                    if (id.sub == 3) { // 13/3 is the join answer, 13/10 the invite answer
                        context.events.onTeamJoinAnswer(answer, otherId);
                    } else {
                        context.events.onTeamInviteAnswer(answer, otherId);
                    }
                    return;
                }
                break;
            }
            case 4: { // 13/4: i32 id, left or disbanded
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onTeamLeft(playerId);
                    return;
                }
                break;
            }
            case 5: { // 13/5: i32 leader, i32 member, joined
                const std::int32_t leaderId = reader.i32();
                const std::int32_t memberId = reader.i32();
                if (!reader.failed()) {
                    context.events.onTeamJoined(leaderId, memberId);
                    return;
                }
                break;
            }
            case 6: // 13/6: the teams on the map (TeamLists)
                context.events.onSceneTeams(protocol::decodeSceneTeams(payload));
                return;
            case 9: { // 13/9: i32 inviter, an invitation
                const std::int32_t inviterId = reader.i32();
                if (!reader.failed()) {
                    context.events.onTeamInvite(inviterId);
                    return;
                }
                break;
            }
            // 13/7 and 13/8: ignored
            case 7:
            case 8: return;
            // 13/11: show the fan icon, 13/12: hide it (lists of i32 player ids)
            case 11:
            case 12: {
                std::vector<std::int32_t> ids;
                while (reader.remaining() >= 4) { // 4 bytes = one i32 id
                    ids.push_back(reader.i32());
                }
                context.events.onTeamFanIcon(ids, id.sub == 11); // 13/11 shows the icon
                return;
            }
            case 14: { // 13/14: u8 reason, the team shape was refused
                const int reason = reader.u8();
                if (!reader.failed()) {
                    context.events.onTeamShapeRefusal(reason);
                    return;
                }
                break;
            }
            case 15: { // 13/15: u8 code, leader hand-over (1 offer, 2 answer, 3 changed)
                const int code = reader.u8();
                if (code == 1) { // code 1: i32 leader, the leader offer
                    const std::int32_t leaderId = reader.i32();
                    if (!reader.failed()) {
                        context.events.onTeamLeaderOffer(leaderId);
                        return;
                    }
                } else if (code == 2) { // code 2: u8 answer, [i32 member when the answer is 2]
                    const int answer = reader.u8();
                    const std::int32_t memberId = answer == 2 ? reader.i32() : 0; // answer 2 carries a member id
                    if (!reader.failed()) {
                        context.events.onTeamLeaderAnswer(answer, memberId);
                        return;
                    }
                } else if (code == 3) { // code 3: i32 new leader, i32 old leader
                    const std::int32_t newLeaderId = reader.i32();
                    const std::int32_t oldLeaderId = reader.i32();
                    if (!reader.failed()) {
                        context.events.onTeamLeaderChanged(newLeaderId, oldLeaderId);
                        return;
                    }
                }
                break;
            }
            case 13: { // 13/13: u8 code, why a team could not be formed
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onTeamJoinError(code);
                    return;
                }
                break;
            }
            default: // any other sub id: unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
