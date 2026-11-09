#include "client/actions/Action52BattleRound.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction52BattleRound(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 52/1: the fighters choose their commands
            context.events.onBattleRound();
        } else {
            context.events.onUnhandled(id);
        }
    }

}
