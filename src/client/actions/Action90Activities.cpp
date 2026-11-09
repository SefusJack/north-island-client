#include "client/actions/Action90Activities.h"

#include "protocol/ActivityLists.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction90Activities(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 0: // 90/0: the event schedule
                if (const auto list = protocol::decodeActivityList(payload)) {
                    context.events.onActivityList(*list);
                    return;
                }
                break;
            case 1: // 90/1: the states of the events
                if (const auto states = protocol::decodeActivityStates(payload)) {
                    context.events.onActivityStates(*states);
                    return;
                }
                break;
            case 2: // 90/2: u8 notice (0 server busy, 1 schedule updated)
                if (payload.size() > 2) { // the notice byte follows the action and sub id
                    context.events.onActivityNotice(payload[2]); // byte 2: the notice code
                    return;
                }
                break;
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
