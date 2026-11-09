#include "client/actions/Action89Exp.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction89Exp(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 89/1: u8 kind, i32 exp an event gave
            core::ByteReader reader(payload, 2); // skip the action and sub id bytes
            const std::uint8_t kind = reader.u8();
            const std::int32_t value = reader.i32();
            if (!reader.failed() && kind == 1) { // kind 1: event exp
                context.events.onEventExp(value);
                return;
            }
        }
        context.events.onUnhandled(id);
    }

}
