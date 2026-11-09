#include "client/actions/Action44Rebirth.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <optional>
#include <string>

namespace client {

    void handleAction44Rebirth(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 2) { // 44/2: open the rebirth form
            context.events.onNpcUpgradeForm();
            return;
        }
        if (id.sub == 1) { // 44/1: u8 rebirth result code
            core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
            const int code = reader.u8();
            std::optional<std::string> text;
            switch (code) {
                case 0: text = "Rebirth Succeeded"; break; // code 0: success
                case 1: text = "Necessary Items for Rebirth"; break; // code 1: items missing
                case 2: text = "No available Pets for Rebirth"; break; // code 2: no pet available
                case 3: text = "Pet's level is less than 30"; break; // code 3: pet level too low
                case 4: text = "Players can't obtain rebirth pets repeatedly in the hotel."; break; // code 4: rebirth pet already in the hotel
                case 5: text = "Players can't obtain rebirth Pets to follow them"; break; // code 5: rebirth pet already following
                case 6: text = "The pet you choose can't grow up"; break; // code 6: pet cannot grow
                case 7: text = "The Friendship Brooch of the pet is less than 60"; break; // code 7: friendship too low
                case 8: text = "The equipment box of the pet is not empty"; break; // code 8: pet equipment box not empty
                case 255: text = "Extra error"; break; // code 255: other error
                default: break; // unknown code: no notice
            }
            if (!reader.failed()) {
                if (text) {
                    context.events.onNoticeMessage(*text, 2000); // notice display time in ms
                }
                return;
            }
        }
        context.events.onUnhandled(id);
    }

}
