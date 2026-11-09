#include "client/actions/Action199Gm.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction199Gm(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        switch (id.sub) {
            case 2: { // 199/2: u32 player, name, text: a player's local line (GM channel 4)
                const std::int32_t player = reader.i32();
                reader.shortText();
                const std::string text = reader.rest();
                if (!reader.failed()) {
                    context.events.onChat(4, player, text); // chat channel 4 = GM
                    return;
                }
                break;
            }
            case 4: // 199/4: u32 player, text: a whisper (channel 3)
            case 6: { // 199/6: u32 player, text: a world line (channel 1)
                const std::int32_t player = reader.i32();
                const std::string text = reader.rest();
                if (!reader.failed()) {
                    context.events.onChat(id.sub == 4 ? 3 : 1, player, text); // 199/4 whisper = channel 3, else world = channel 1
                    return;
                }
                break;
            }
            default: // any other sub id (mail, player list, announcement arms are not built): unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
