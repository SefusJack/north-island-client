#include "client/actions/Action27Shop.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction27Shop(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: { // 27/1: u8 how a purchase went
                const int result = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onStoreBought(result);
                break;
            }
            case 2: { // 27/2: u8 how a sale went
                const int result = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onStoreSold(result);
                break;
            }
            case 3: // 27/3: open the sale form for goods
                context.events.onSaleOpened(true);
                break;
            case 4: // 27/4: open the sale form for equipment
                context.events.onSaleOpened(false);
                break;
            case 5: { // 27/5: u8 Lucky 7 count
                const int count = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onLucky7Count(count);
                break;
            }
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
