#include "client/actions/Action54Christmas.h"

namespace client {

    namespace {

        const char* const kBegins = "From December 17, 2am to December 31, 2am EST (G M T-5) only, eliminate monsters levels 10 or higher for a chance to "
                                    "acquire 3 special Yuletide bells! Collect these bells and exchange it with Santa Claus for a sock-load of surprises!";
        const char* const kEnds = "The Christmas special has ended. Santa Claus and the Christmas Tree will be removed at the next server maintenance. "
                                  "Please make your exchange before this period. Thank you and Merry Christmas to you and your family!";

    }

    void handleAction54Christmas(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 54/1: event begins: announce in chat and turn on
                context.events.onChristmasAnnounced(kBegins);
                context.events.onChristmasEvent(true);
                return;
            case 2: // 54/2: event ends: announce in chat and turn off
                context.events.onChristmasAnnounced(kEnds);
                context.events.onChristmasEvent(false);
                return;
            case 3: // 54/3: turn the event on (no chat line)
            case 4: // 54/4: turn the event off (no chat line)
                context.events.onChristmasEvent(id.sub == 3); // sub 3 = on
                return;
            case 5: // 54/5: announce the beginning only
                context.events.onChristmasAnnounced(kBegins);
                return;
            case 6: // 54/6: announce the end only
                context.events.onChristmasAnnounced(kEnds);
                return;
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
