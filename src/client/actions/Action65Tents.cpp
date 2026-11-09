#include "client/actions/Action65Tents.h"

namespace client {

    void handleAction65Tents(const core::Bytes& payload, MessageContext& context) { context.events.onTentMessage(payload); }

}
