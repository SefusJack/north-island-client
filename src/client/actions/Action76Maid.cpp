#include "client/actions/Action76Maid.h"

namespace client {

    void handleAction76Maid(const core::Bytes& payload, MessageContext& context) { context.events.onMaidAction(payload); }

}
