#include "client/actions/Action00Kick.h"

#include "core/Log.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction00Kick(const core::Bytes& payload, MessageContext& context) {
        const int reason = protocol::MessageId::of(payload).sub;
        core::logInfo("server 0/", reason, ": the server ends the session");
        context.connection.send({protocol::kActionHello});
        context.events.onKicked(reason);
    }

}
