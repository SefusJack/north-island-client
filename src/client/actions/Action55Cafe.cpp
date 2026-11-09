#include "client/actions/Action55Cafe.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction55Cafe(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 55/1: open the cafe number form
            context.events.onCafeIdForm();
            return;
        }
        if (id.sub == 2) { // 55/2: u8 answer to the cafe number
            core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
            const int code = reader.u8();
            if (!reader.failed()) {
                if (code == 1) { // code 1: accepted
                    context.events.onNoticeMessage("Succeed to enter", 2000); // notice display time in ms
                    return;
                }
                if (code == 2) { // code 2: refused
                    context.events.onNoticeMessage("Fail to enter", 2000); // notice display time in ms
                    context.events.onCafeIdForm();
                    return;
                }
            }
        }
        context.events.onUnhandled(id);
    }

}
