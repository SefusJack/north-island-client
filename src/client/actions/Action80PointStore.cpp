#include "client/actions/Action80PointStore.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction80PointStore(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 80/1: the rows, opens the form
            case 2: // 80/2: not enough points
            case 3: // 80/3: a line bought
            case 4: // 80/4: points taken (u16 item, i32 points, u8 count)
            case 5: // 80/5: points taken (i32 points)
            case 6: // 80/6: points taken (i32 points)
            case 7: // 80/7: not enough points
            case 8: // 80/8: the server is busy
                context.events.onPointStoreMessage(payload);
                return;
            case 21: // 80/21: Obtain list / birth check form data, ignored
            case 22: // 80/22: Obtain list / birth check form data, ignored
            case 23: // 80/23: Obtain list / birth check form data, ignored
            case 31: // 80/31: Obtain list / birth check form data, ignored
                return;
            case 41: // 80/41: RecvPlayerTripleFlags, ignored
                return;
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
