#include "client/actions/Action68ItemBag.h"

#include "core/ByteReader.h"
#include "protocol/ItemLists.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction68ItemBag(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: // 68/1: order done, cell a to bag place b
            case 2: // 68/2: order done, cell a to cell b
            case 3: { // 68/3: order done, bag place a to cell b
                const int first = reader.u8();
                const int second = reader.u8();
                const int count = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onItemBagMoved(id.sub, first, second, count);
                break;
            }
            case 4: // 68/4: the whole grid (29-byte records)
                context.events.onItemBagList(protocol::decodeBagItems(payload));
                break;
            case 5: // 68/5: ignored
            case 6: // 68/6: ignored
            case 7: // 68/7: ignored
            case 8: // 68/8: ignored
                break;
            case 9: // 68/9: show the form
                context.events.onItemBagOpened();
                break;
            default: // unknown sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
