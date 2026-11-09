#include "client/actions/Action71Machines.h"

namespace client {

    void handleAction71Machines(const core::Bytes& payload, MessageContext& context) { context.events.onMachineMessage(payload); }

}
