#include "client/actions/Action04PlayerPresent.h"

#include "protocol/OtherPlayerRecord.h"

namespace client {

    void handleAction04PlayerPresent(const core::Bytes& payload, MessageContext& context) {
        if (const auto record = protocol::OtherPlayerRecord::decode(payload)) {
            context.events.onPlayerAppeared(*record, false);
        }
    }

}
