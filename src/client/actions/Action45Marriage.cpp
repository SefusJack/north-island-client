#include "client/actions/Action45Marriage.h"

namespace client {

    void handleAction45Marriage(const core::Bytes& payload, MessageContext& context) { context.events.onMarriageMessage(payload); }

}
