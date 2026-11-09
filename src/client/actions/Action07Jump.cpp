#include "client/actions/Action07Jump.h"

#include "core/ByteReader.h"

namespace client {

    void handleAction07Jump(const core::Bytes& payload, MessageContext& context) {
        core::ByteReader reader(payload, 1);
        const std::int32_t player = reader.i32();
        // 2 bytes: the u16 map id
        reader.skip(2);
        const int x = reader.u16();
        const int y = reader.u16();
        if (!reader.failed()) {
            context.events.onPlayerJumped(player, core::Point{x, y});
        }
    }

}
