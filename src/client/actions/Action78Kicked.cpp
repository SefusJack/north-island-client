#include "client/actions/Action78Kicked.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction78Kicked(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 78/1: player has been kicked out
            context.events.onNoticeMessage("Player has been kicked out", 1500); // 1500: notice duration in ms
            return;
        }
        context.events.onUnhandled(id);
    }

}
