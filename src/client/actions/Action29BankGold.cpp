#include "client/actions/Action29BankGold.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction29BankGold(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: // 29/1: i32 gold put into the account
            case 2: // 29/2: i32 gold taken out
            case 4: { // 29/4: i32 the gold in the account
                const std::int32_t amount = reader.i32();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                if (id.sub == 4) { // 29/4: absolute amount
                    context.events.onBankGold(amount);
                } else {
                    context.events.onBankGoldMoved(id.sub == 1 ? amount : -amount); // 29/1 deposit is positive
                }
                break;
            }
            case 3: // 29/3: the account is full
                context.events.onBankFull();
                break;
            case 5: // 29/5: open the money account form
            case 6: // 29/6: open the bank item form
                context.events.onBankOpened(id.sub == 6); // sub 6 = item form
                break;
            case 7: // 29/7: the money form may take a withdrawal again
            case 8: // 29/8: the money form may take a deposit again
                context.events.onBankGoldDone(id.sub == 8); // sub 8 = deposit
                break;
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
