#include "client/actions/Action104Lottery.h"

#include "protocol/LotteryLists.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction104Lottery(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        // 104/1: lottery packet; payload[2] is the second sub id and needs at least 3 bytes
        if (id.sub == 1 && payload.size() > 2) {
            if (payload[2] == 1) { // 104/1/1: the online draw's prizes and the draws used
                if (const auto info = protocol::decodeLotteryInfo(payload)) {
                    context.events.onLotteryInfo(*info);
                    return;
                }
            } else if (payload[2] == 2) { // 104/1/2: the prize won
                if (const auto result = protocol::decodeLotteryResult(payload)) {
                    context.events.onLotteryResult(*result);
                    return;
                }
            }
        }
        context.events.onUnhandled(id);
    }

}
