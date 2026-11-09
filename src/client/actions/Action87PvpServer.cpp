#include "client/actions/Action87PvpServer.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction87PvpServer(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 87/1: u8 mode, relogin to the PvP server or back
            core::ByteReader reader(payload, 2); // skip the action and sub id bytes
            const int mode = reader.u8();
            if (reader.failed() || (mode != 1 && mode != 2)) { // mode 1: to the PvP server, 2: back
                context.events.onUnhandled(id);
                return;
            }
            context.events.onServerRelogin(mode == 1);
            return;
        }
        if (id.sub == 2) { // 87/2: u8 code, the refusal of the move
            core::ByteReader reader(payload, 2); // skip the action and sub id bytes
            const int code = reader.u8();
            if (reader.failed()) {
                context.events.onUnhandled(id);
                return;
            }
            const char* const busy = "The server is busy, please try later";
            switch (code) {
                case 2: // code 2: function not usable on a PvP server
                    context.events.onNoticeMessage("Cannot use this function on a PvP Server", 2000); // 2000: notice duration in ms
                    return;
                case 3: // code 3: the PvP times
                    context.events.onNoticeMessage(
                        "Time of PvP on the PvP Server: \rEvery Saturday, from 4: 00pm until 6: 00pm\rEvery Sunday, from  4: 00pm until 6: 00pm",
                        2000
                    ); // 2000: notice duration in ms
                    return;
                case 4: // code 4: PvP paused today
                    context.events.onNoticeMessage("The PvP on the PvP Server will be pause today", 2000); // 2000: notice duration in ms
                    return;
                default: // any other code: server busy
                    context.events.onNoticeMessage(busy, 2000); // 2000: notice duration in ms
                    return;
            }
        }
        if (id.sub == 3) { // 87/3: RecvServerCode, ignored
            return;
        }
        context.events.onUnhandled(id);
    }

}
