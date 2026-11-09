#include "client/actions/Action84Top100.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction84Top100(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 84/1: show the Top 100 window
                context.events.onTop100Opened();
                return;
            case 2: // 84/2: one of the Top 100 lists
                context.events.onTop100List(payload);
                return;
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
