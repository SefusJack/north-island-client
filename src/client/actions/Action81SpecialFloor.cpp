#include "client/actions/Action81SpecialFloor.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction81SpecialFloor(const core::Bytes& payload, MessageContext& context) {
        constexpr int kBattleTimeLimit = 3; // 81/3: start the battle's time limit
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == kBattleTimeLimit) {
            context.events.onBattleTimeLimit();
            return;
        }
        if (id.sub == 1 || id.sub == 2 || id.sub == 4) { // 81/1, 81/2, 81/4: things of a special house room
            context.events.onSpecFloorMessage(payload);
            return;
        }
        context.events.onUnhandled(id);
    }

}
