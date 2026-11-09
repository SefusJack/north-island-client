#include "client/actions/Action33Switches.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <array>

namespace client {

    void handleAction33Switches(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: { // 33/1: u8 why watching or fighting a player was refused
                const int reason = reader.u8();
                if (!reader.failed()) {
                    context.events.onPlayerRefused(reason);
                    return;
                }
                break;
            }
            case 2: { // 33/2: u8 PK, fighting, trade, party switches, u8 chat channel bits
                std::array<int, 4> switches{}; // 4 switches: PK, fighting, trade, party
                for (int& state : switches) {
                    state = reader.u8();
                }
                const int channels = reader.u8();
                if (!reader.failed()) {
                    context.events.onSwitches(switches, channels);
                    return;
                }
                break;
            }
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
