#include "client/actions/Action34Subscription.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    void handleAction34Subscription(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 34/1: u16 IM points
            core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
            const std::uint16_t points = reader.u16();
            if (!reader.failed()) {
                context.events.onNoticeMessage("Your Wonderland Online IM Points : \r" + std::to_string(points) + " points", 2000); // notice display time in ms
                return;
            }
        }
        context.events.onUnhandled(id);
    }

}
