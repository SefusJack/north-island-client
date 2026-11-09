#include "client/actions/Action43Alliance.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <optional>
#include <string>

namespace client {

    void handleAction43Alliance(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: // 43/1: alliance request (RecvAllyRequest)
            case 2: // 43/2: alliance answer (RecvAllyAnswer)
            case 3: // 43/3: alliance disbanded (RecvAllyDisband)
            case 4: // 43/4: alliance list (RecvAllyList)
            case 5: // 43/5: left the alliance (RecvAllyLeft)
                context.events.onAllianceMessage(payload);
                return;
            case 6: { // 43/6: u8 code: the Trojan War refuses an alliance action
                const int code = reader.u8();
                std::optional<std::string> text;
                if (code == 1) { // code 1: alliance refused
                    text = "Unable to be allied with others during the Trojan War.";
                } else if (code == 2) { // code 2: answer refused
                    text = "Unable to respond to other's invitation during the Trojan War.";
                } else if (code == 3) { // code 3: leave refused
                    text = "Unable to leave allies during the Trojan War.";
                }
                if (!reader.failed()) {
                    if (text) {
                        context.events.onNoticeMessage(*text, 2000); // notice display time in ms
                    }
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
