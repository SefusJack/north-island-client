#include "client/actions/Action40HotKeys.h"

#include "protocol/HotKeyRequests.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction40HotKeys(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 40/1: the F1-F8 bar's places
            context.events.onHotKeys(protocol::decodeHotKeys(payload));
            return;
        }
        context.events.onUnhandled(id);
    }

}
