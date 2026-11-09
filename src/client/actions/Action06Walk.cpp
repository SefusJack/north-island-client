#include "client/actions/Action06Walk.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction06Walk(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        switch (id.sub) {
            case 1: { // 6/1: i32 id, u8 direction, u16 x, u16 y: walks there then faces that way
                const std::int32_t player = reader.i32();
                const int direction = reader.u8();
                const int x = reader.u16();
                const int y = reader.u16();
                if (!reader.failed()) {
                    context.events.onPlayerWalks(player, core::Point{x, y}, direction);
                }
                break;
            }
            case 2: { // 6/2: u8 lock, the own player stops
                const int lock = reader.u8();
                if (!reader.failed()) {
                    context.events.onWalkStopped(lock != 0);
                }
                break;
            }
            case 3: { // 6/3: i32 id, u16 x, u16 y: walks there
                const std::int32_t player = reader.i32();
                const int x = reader.u16();
                const int y = reader.u16();
                if (!reader.failed()) {
                    context.events.onPlayerWalks(player, core::Point{x, y}, std::nullopt);
                }
                break;
            }
            default: // any other sub id: unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
