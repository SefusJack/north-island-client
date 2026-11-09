#include "client/actions/Action10Names.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction10Names(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        const std::int32_t playerId = reader.i32();
        if (id.sub == 1) { // 10/1: i32 player, pstr8 nickname
            const std::string nickname = reader.shortText();
            if (!reader.failed()) {
                context.events.onNickname(playerId, nickname);
                return;
            }
        } else if (id.sub == 2 || id.sub == 3) { // 10/2 and 10/3: i32 player, u8 heart number
            const int heartNumber = reader.u8();
            if (!reader.failed()) {
                context.events.onHeartNumber(playerId, heartNumber);
                return;
            }
        }
        context.events.onUnhandled(id);
    }

}
