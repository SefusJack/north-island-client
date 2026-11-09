#include "client/actions/Action62Furniture.h"

namespace client {

    void handleAction62Furniture(const core::Bytes& payload, MessageContext& context) { context.events.onFurnitureMessage(payload); }

}
