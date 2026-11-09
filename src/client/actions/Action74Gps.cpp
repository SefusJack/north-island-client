#include "client/actions/Action74Gps.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    void handleAction74Gps(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // skip the action and sub id bytes
        switch (id.sub) {
            case 1: { // 74/1: u8 point, u16 map, u16 x, u16 y: a recorded place
                const int point = reader.u8();
                const int map = reader.u16();
                const int x = reader.u16();
                const int y = reader.u16();
                if (reader.failed()) {
                    break;
                }
                context.events.onGpsMark(point, map, x, y);
                return;
            }
            case 2: // 74/2: i32 player, arrival light again; the port draws none
                return;
            case 3: { // 74/3: u8 kind (+ u16 minutes when 1): Dreaming Pill notice
                const int kind = reader.u8();
                const int minutes = kind == 1 ? reader.u16() : 0; // kind 1: minutes left follow
                if (reader.failed()) {
                    break;
                }
                constexpr int kMs = 1500; // notice duration in ms
                context.events.onDreamingPill(kind);
                if (kind == 1) {
                    // 60 minutes per hour
                    const std::string left = minutes >= 60 ? std::to_string(minutes / 60) + " Hour " + std::to_string(minutes % 60) : std::to_string(minutes);
                    context.events.onNoticeMessage("Remaining Time of Dreaming Pill effect\xA1G" + left + " Minute ", kMs);
                } else if (kind == 3) { // kind 3: expired
                    context.events.onNoticeMessage("The effective time of Dreaming Pill is expired", kMs);
                } else if (kind == 4) { // kind 4: in use
                    context.events.onNoticeMessage("The pill is being used", kMs);
                }
                return;
            }
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
