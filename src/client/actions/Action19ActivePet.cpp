#include "client/actions/Action19ActivePet.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction19ActivePet(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 2) { // 19/2: no fighting pet (ClearActivePet)
            context.events.onActivePet(0, false);
            return;
        }
        if (id.sub == 6) { // 19/6: u8 place, u8 dead flag: a city battle pet
            core::ByteReader reader(payload, 2);
            const int place = reader.u8();
            const bool dead = reader.u8() != 0;
            if (!reader.failed()) {
                if (place >= 1 && place <= 4) { // pet places 1..4
                    context.events.onCityBattleDeadPet(place, dead);
                }
                return;
            }
        }
        if (id.sub == 7) { // 19/7: i32 player, another's pet rests
            core::ByteReader reader(payload, 2);
            const std::int32_t playerId = reader.i32();
            if (!reader.failed()) {
                context.events.onScenePetRests(playerId);
                return;
            }
        }
        if (id.sub == 1 || id.sub == 4) { // 19/1: i32 npc, the fighting pet (RecvActivePet); 19/4: the same at login (RecvSelectedPet)
            core::ByteReader reader(payload, 2);
            const std::int32_t npcId = reader.i32();
            if (!reader.failed()) {
                context.events.onActivePet(npcId, id.sub == 4);
                return;
            }
        }
        context.events.onUnhandled(id);
    }

}
