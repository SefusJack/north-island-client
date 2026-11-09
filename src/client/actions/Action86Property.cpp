#include "client/actions/Action86Property.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction86Property(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 86/1: the lost property list (RecvLostPropertyList)
            case 2: // 86/2: a piece reclaimed (RecvReclaimDone)
            case 3: // 86/3: a refusal (RecvReclaimError)
                context.events.onPropertyMessage(payload);
                break;
            default: // unknown sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
