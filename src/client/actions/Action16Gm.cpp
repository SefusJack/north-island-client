#include "client/actions/Action16Gm.h"

#include "core/ByteReader.h"
#include "protocol/GmQuestion.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction16Gm(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2);
        switch (id.sub) {
            case 1: { // 16/1: the GM's question with a choice list (TAC_Question)
                if (auto question = protocol::decodeGmQuestion(payload)) {
                    context.events.onGmQuestion(question->kind, question->question, question->choices);
                    return;
                }
                break;
            }
            case 18: // 16/18: u32 id (not read), text: a GM's system line (RecvGmSystemMsg)
                // 6 = action, sub id and the u32 id before the text
                if (payload.size() > 6) {
                    context.events.onChat(0, 0, std::string(payload.begin() + 6, payload.end()));
                    return;
                }
                break;
            case 2: { // 16/2: u8 kind, u8 code: the notice of the answer to the GM's question
                const int kind = reader.u8();
                const int code = reader.u8();
                // kind 1 = answer notice; code 1 = right answer; 1200 = notice duration in ms
                if (!reader.failed() && kind == 1) {
                    context.events.onNoticeMessage(code == 1 ? "You are smart. You got the right answer." : "You got the wrong answer.", 1200);
                    return;
                }
                break;
            }
            case 7: { // 16/7: u8 kind, u8 channel bits, u32 minutes: a GM silences channels (RecvChatSilence)
                const int kind = reader.u8();
                const int mask = reader.u8();
                const std::uint32_t minutes = reader.u32();
                if (!reader.failed()) {
                    context.events.onChatSilenced(kind, mask, minutes);
                    return;
                }
                break;
            }
            case 8: // 16/8: the GM releases the mute
                context.events.onChatSilenceReleased(true);
                context.events.onChat(0, 0, "release from mute");
                return;
            case 13: // 16/13: release from mute
                context.events.onChat(0, 0, "Release from mute");
                context.events.onChatSilenceReleased(false);
                return;
            case 20: // 16/20: "You talk too often"
                context.events.onChat(0, 0, "You talk too often. Please take break and talk later.");
                return;
            // 16/200: the qol server's GM status, not an original sub id
            case 200: {
                const int flags = reader.u8();
                if (!reader.failed()) {
                    context.events.onGmStatus(flags);
                    return;
                }
                break;
            }
            default: // any other sub id: unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
