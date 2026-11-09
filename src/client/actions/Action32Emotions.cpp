#include "client/actions/Action32Emotions.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction32Emotions(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        if (id.sub == 1 || id.sub == 3) { // 32/1 and 32/3: i32 player, u8 emotion
            const std::int32_t playerId = reader.i32();
            const int emotionId = reader.u8();
            if (!reader.failed()) {
                context.events.onEmotion(playerId, emotionId);
                return;
            }
        }
        if (id.sub == 2) { // 32/2: records of i32 player, u8 sprite slot
            while (reader.remaining() >= 5) { // record size: i32 + u8
                const std::int32_t playerId = reader.i32();
                const int slot = reader.u8();
                context.events.onAction(playerId, slot);
            }
            return;
        }
        context.events.onUnhandled(id);
    }

}
