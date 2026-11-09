#include "client/actions/Action09CreateCharacter.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction09CreateCharacter(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 3: { // 9/3: u8 code, answer to the name check
                core::ByteReader reader(payload, 2);
                switch (reader.u8()) {
                    case 0: // name accepted
                        context.events.onNameChecked(NameCheckResult::Accepted);
                        break;
                    case 1: // name taken
                        context.events.onNameChecked(NameCheckResult::Unavailable);
                        break;
                    case 2: // name invalid
                        context.events.onNameChecked(NameCheckResult::Invalid);
                        break;
                    // any other code: ignored
                    default: break;
                }
                break;
            }
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
