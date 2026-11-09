#include "client/actions/Action22MapNpc.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction22MapNpc(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 12) { // 22/12: u8 kind (1 NPC, 2 door, 3 mine), u16 index, u8 sign: sign on the small map
            core::ByteReader reader(payload, 2);
            const int kind = reader.u8();
            const int index = reader.u16();
            const int sign = reader.u8();
            if (!reader.failed()) {
                context.events.onMapSign(kind, index, sign);
            }
            return;
        }
        if (id.sub == 4) { // 22/4: the map NPCs' state list (RevMapNpcStateList)
            constexpr std::size_t kRecord = 14; // bytes per NPC state record: u16 index, u8 hold frame, u8, u16 x, u16 y, u8 state, u32 hide ms, u8 action
            // records start after the action byte (1); each record is kRecord bytes plus the sub id byte (1) in the bounds check
            for (std::size_t first = 1; first + 1 + kRecord <= payload.size(); first += kRecord) {
                core::ByteReader record(payload, first + 1);
                const int index = record.u16();
                const int holdFrame = record.u8();
                record.u8();
                const int x = record.u16();
                const int y = record.u16();
                const int state = record.u8();
                const std::uint32_t hideMs = record.u32();
                const int action = record.u8();
                if (!record.failed()) {
                    context.events.onNpcState(index, holdFrame, core::Point{x, y}, state, hideMs, action);
                }
            }
            return;
        }
        core::ByteReader reader(payload, 2);
        const int index = reader.u16();
        switch (id.sub) {
            case 1: { // 22/1: u8 frame, a scene object shows that frame still (255 animates)
                const int frame = reader.u8();
                if (!reader.failed()) {
                    context.events.onNpcFrame(index, frame);
                }
                break;
            }
            case 2: // 22/2: u16 x, u16 y, u8 speed: walks there
            case 5: { // 22/5: u16 x, u16 y: stands there
                const int x = reader.u16();
                const int y = reader.u16();
                if (!reader.failed()) {
                    context.events.onNpcMoves(index, core::Point{x, y}, id.sub == 2); // sub 2 walks there, sub 5 stands there
                }
                break;
            }
            case 6: { // 22/6: u8 state, shown unless the state is 2 or 4
                const int state = reader.u8();
                if (!reader.failed()) {
                    context.events.onNpcShowState(index, state);
                }
                break;
            }
            case 3: // 22/3: the same as 22/10 (seconds in four bytes)
            case 10: // 22/10: u16 seconds, the NPC is hidden
            case 11: { // 22/11: u16 seconds, the NPC is shown
                const int seconds = id.sub == 3 ? reader.i32() & 0xFFFF : reader.u16(); // 0xFFFF keeps the low 16 bits (the first two bytes the game reads)
                if (!reader.failed()) {
                    if (id.sub == 11) { // 22/11 reveals, else 22/10 or 22/3 hides
                        context.events.onNpcRevealed(index, seconds);
                    } else {
                        context.events.onNpcHidden(index, seconds);
                        if (id.sub == 3) { // 22/3 also makes the NPC vanish
                            context.events.onNpcVanished(index);
                        }
                    }
                }
                break;
            }
            case 7: { // 22/7: u8 emotion, the NPC shows emotion 100 + n
                const int emotion = reader.u8();
                if (!reader.failed()) {
                    context.events.onNpcEmotion(index, emotion + 100); // emotion ids of NPCs start at 100
                }
                break;
            }
            case 8: { // 22/8: u8 action, the NPC plays an action (HandleNpcActionPacket)
                const int action = reader.u8();
                if (!reader.failed()) {
                    context.events.onNpcAction(index, action, 255, true); // 255 = no frame held, the action plays
                }
                break;
            }
            case 9: { // 22/9: u8 action, u8 frame held (HandleNpcActionDirPacket)
                const int action = reader.u8();
                const int holdFrame = reader.u8();
                if (!reader.failed()) {
                    context.events.onNpcAction(index, action, holdFrame, false);
                }
                break;
            }
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
