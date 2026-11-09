#include "client/actions/Action57Sports.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction57Sports(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: { // 57/1: a mini game starts (RecvSportStart)
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int kind = reader.u8();
                const int npcId = reader.u16();
                const int level = reader.u8();
                const int scene = reader.u8();
                // 3: header (2) plus at least the kind byte
                if (payload.size() >= 3) {
                    context.events.onSportStart(kind, npcId, level, scene);
                    return;
                }
                break;
            }
            case 2: context.events.onSportEnd(); return; // 57/2: the mini game ends
            case 8: context.events.onQuizMessage(payload); return; // 57/8: quiz game message
            case 9: context.events.onPokeMessage(payload); return; // 57/9: poke game message
            case 10: context.events.onCompassMessage(payload); return; // 57/10: compass game message
            case 11: context.events.onGobangMessage(payload); return; // 57/11: gobang game message
            case 12: context.events.onWheelMessage(payload); return; // 57/12: Lucky Wheel list and answers
            case 3: { // 57/3: u8 who: the game is not available
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int who = reader.u8();
                if (!reader.failed() && (who == 1 || who == 2)) { // who: 1 everybody, 2 this player
                    context.events.onNoticeMessage(
                        who == 1 ? "This function is unavailable at present" : "This playerThis function is unavailable at present",
                        1200
                    ); // notice display time in ms
                    return;
                }
                break;
            }
            default: break; // any other sub id: reported as unhandled
        }
        context.events.onUnhandled(id);
    }

}
