#include "client/actions/Action63Account.h"

#include "core/ByteReader.h"
#include "protocol/CharacterList.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction63Account(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 63/1: the character list
                if (const auto list = protocol::CharacterList::decode(payload)) {
                    context.events.onCharacterList(*list);
                }
                break;
            case 2: { // 63/2: u32 login id
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                context.events.onLoginId(reader.u32());
                break;
            }
            default: // unknown sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
