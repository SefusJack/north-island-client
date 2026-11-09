#include "client/actions/Action72BlackBoard.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction72BlackBoard(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1 || id.sub == 2) { // 72/1 the messages, 72/2 refusal of a write
            context.events.onBlackBoardMessage(payload);
            return;
        }
        context.events.onUnhandled(id);
    }

}
