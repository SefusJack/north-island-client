#include "client/actions/Action03PlayerEnters.h"

#include "core/ByteReader.h"
#include "protocol/OtherPlayerRecord.h"
#include "protocol/OwnPlayerRecord.h"
#include "protocol/PlayerId.h"

namespace client {

    void handleAction03PlayerEnters(const core::Bytes& payload, MessageContext& context) {
        core::ByteReader reader(payload, 1);
        const std::int32_t id = reader.i32();
        if (static_cast<std::uint32_t>(protocol::stripServerOffset(id)) == context.ownPlayerId) {
            if (const auto record = protocol::OwnPlayerRecord::decode(payload)) {
                context.events.onOwnPlayerEntered(*record);
            }
        } else if (const auto record = protocol::OtherPlayerRecord::decode(payload)) {
            context.events.onPlayerAppeared(*record, true);
        }
    }

}
