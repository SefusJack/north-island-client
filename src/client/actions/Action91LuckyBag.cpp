#include "client/actions/Action91LuckyBag.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction91LuckyBag(const core::Bytes& payload, MessageContext& context) {
        constexpr int kInfo = 2; // 91/2: what a bag can hold
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == kInfo) {
            context.events.onLuckyBagMessage(payload);
            return;
        }
    }

}
