#include "client/actions/Action85Instances.h"

namespace client {

    void handleAction85Instances(const core::Bytes& payload, MessageContext& context) { context.events.onInstanceMessage(payload); }

}
