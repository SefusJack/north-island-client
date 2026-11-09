#include "client/actions/Action21IdAndPw.h"

#include "protocol/MessageId.h"

namespace client {

    void handleAction21IdAndPw(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1 || id.sub == 2) { // 21/1 and 21/2: open the second ID and password form in that mode
            context.events.onIdAndPwForm(id.sub);
            return;
        }
        if (id.sub == 3) { // 21/3: the account or password was refused
            context.events.onNoticeMessage("Invalid account or password", 1500); // notice duration in ms
            return;
        }
        context.events.onUnhandled(id);
    }

}
