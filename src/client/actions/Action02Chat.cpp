#include "client/actions/Action02Chat.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    void handleAction02Chat(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        // 2/0..7: chat channel 0 system .. 7 ally (the sub code); 2/11 and 2/14: system lines
        if (id.sub <= 7 || id.sub == 11 || id.sub == 14) {
            const std::int32_t speaker = reader.i32();
            if (!reader.failed()) {
                context.events.onChat(id.sub <= 7 ? id.sub : 0, speaker, reader.rest()); // highest chat channel sub code (7 ally)
            }
        } else if (id.sub == 12) { // 2/12: text from the system (speaker 0)
            context.events.onChat(0, 0, reader.rest());
        } else if (id.sub == 15) { // 2/15: system announcement (RecvSystemAnnounce)
            SystemAnnounce announce;
            announce.kind = reader.u8();
            switch (announce.kind) {
                case 0: // announce kind 0: plain text
                    announce.text = reader.rest();
                    break;
                case 1: // announce kind 1: source id, prize id, count, player
                    announce.sourceId = reader.u16();
                    announce.prizeId = reader.u16();
                    announce.count = reader.u8();
                    announce.player = reader.rest();
                    break;
                case 2: // announce kind 2: source id, prize id, player
                    announce.sourceId = reader.u16();
                    announce.prizeId = reader.u16();
                    announce.player = reader.rest();
                    break;
                case 3: // announce kind 3: machine, prize id, count, player
                    announce.machine = reader.u8();
                    announce.prizeId = reader.u16();
                    announce.count = reader.u8();
                    announce.player = reader.rest();
                    break;
                case 4: // announce kind 4: prize id, count, player
                    announce.prizeId = reader.u16();
                    announce.count = reader.u8();
                    announce.player = reader.rest();
                    break;
                default: // unknown announce kind: ignored
                    return;
            }
            if (!reader.failed()) {
                context.events.onSystemAnnounce(announce);
            }
        } else if (id.sub == 10) { // 2/10: i32 speaker, name, u8 flags, text (a chat line with a name)
            const std::int32_t speaker = reader.i32();
            const std::string name = reader.shortText();
            const int flags = reader.u8();
            const std::string text = reader.rest();
            if (!reader.failed()) {
                context.events.onChatLine(speaker, name, flags, text);
            }
        } else if (id.sub == 8) { // 2/8: chat blocked
            context.events.onChatBlocked();
        } else if (id.sub == 13) { // 2/13: u8 channel the sender may not command is off (ShowChannelOffMsg)
            static const char* const kChannels[] = {"", "Full", "Regional", "Private", "", "Team", "Party"};
            const int channel = reader.u8();
            // valid channel codes are 1..6 (kChannels index); 4 has no name
            if (!reader.failed() && channel >= 1 && channel <= 6 && channel != 4) {
                context.events.onNoticeMessage(std::string(kChannels[channel]) + " channel off. Unable to give orders", 2000); // notice duration in ms
            }
        } else {
            context.events.onUnhandled(id);
        }
    }

}
