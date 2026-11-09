#include "client/actions/Action66Reborn.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction66Reborn(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 11: // 66/11: Wishing God wish message, goes to the reborn form
            case 12: // 66/12: Wishing God wish message
            case 13: // 66/13: Wishing God wish message
            case 14: // 66/14: Wishing God wish message
            case 15: // 66/15: Wishing God wish message
            case 16: // 66/16: Wishing God wish message
                context.events.onRebornWishMessage(payload);
                return;
            default: // other sub ids (incl. 66/1, 66/2): reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
