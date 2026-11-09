#include "client/actions/Action79Career.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction79Career(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 79/1: open the class form
            context.events.onCareerForm();
            return;
        }
        if (id.sub == 2) { // 79/2: u8 answer to the class choice
            core::ByteReader reader(payload, 2); // skip the action and sub id bytes
            const int code = reader.u8();
            if (!reader.failed()) {
                context.events.onCareerResult(code);
                return;
            }
        }
        context.events.onUnhandled(id);
    }

}
