#include "client/actions/Action26Gold.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction26Gold(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 3) { // 26/3: the money bag is full
            context.events.onNoticeMessage("Your money bag is full.", 2000); // notice display time in ms
            return;
        }
        if (id.sub == 11) { // 26/11: the deposit limit is reached
            context.events.onNoticeMessage("Reached the max deposit limit amount", 2000); // notice display time in ms
            return;
        }
        if (id.sub == 7) { // 26/7: stall trade failure
            core::ByteReader failure(payload, 2); // body starts after the 2-byte action/sub header
            if (failure.u8() == 1 && !failure.failed()) { // failure code 1: show the notice
                context.events.onNoticeMessage("Fail to trade in the stall", 1500); // notice display time in ms
            }
            return;
        }
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        const std::int32_t amount = reader.i32();
        if (reader.failed()) {
            context.events.onUnhandled(id);
            return;
        }
        switch (id.sub) {
            case 1: // 26/1: i32 gold gained
                context.events.onGoldChanged(amount, false);
                break;
            case 2: // 26/2: i32 gold spent
                context.events.onGoldChanged(-amount, false);
                break;
            case 4: // 26/4: i32 the amount now
                context.events.onGoldChanged(amount, true);
                break;
            case 5: // 26/5: sale account gold added
            case 6: // 26/6: sale account gold subtracted
                context.events.onSaleGoldChanged(amount, id.sub == 5); // sub 5 = added
                break;
            case 8: // 26/8: ATM gold, i32 amount then i32 peer player
            case 9: { // 26/9: ATM gold, i32 amount then i32 peer player
                const std::int32_t peerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onAtmBalance(id.sub, amount, peerId);
                } else {
                    context.events.onUnhandled(id);
                }
                break;
            }
            case 10: // 26/10: ATM gold without a peer player
                context.events.onAtmBalance(id.sub, amount, 0);
                break;
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
