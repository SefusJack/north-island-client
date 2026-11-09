#include "client/actions/Action64Manufacture.h"

namespace client {

    void handleAction64Manufacture(const core::Bytes& payload, MessageContext& context) { context.events.onManufactureMessage(payload); }

}
