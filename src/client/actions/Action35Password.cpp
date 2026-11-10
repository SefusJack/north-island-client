#include "client/actions/Action35Password.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction35Password(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: { // 35/1: u8 password change result
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onPasswordChanged(result);
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            case 2: { // 35/2: u8 result, u8 slot: character deletion
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int result = reader.u8();
                const int slot = reader.u8();
                context.events.onCharacterDeleted(result, slot);
                break;
            }
            case 3: { // 35/3: u8 recharge card result
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int result = reader.u8();
                static const char* const kTexts[] =
                    {"Succeed to recharge", "Fail to recharge", "wrong card number", "wrong password", "The card is used.", "You have recharged the points."};
                if (!reader.failed() && result >= 1 && result <= 6) { // results 1..6 map to the six texts
                    context.events.onNoticeMessage(kTexts[result - 1], 2000); // notice display time in ms
                }
                break;
            }
            case 4: { // 35/4: i32 + i32 the account's IM points
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const std::int32_t points = reader.i32();
                const std::int32_t more = reader.i32();
                if (!reader.failed()) {
                    context.events.onImPoints(points + more);
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            case 6: // 35/6: open the refill form
                context.events.onReFillForm(true);
                break;
            case 11: // 35/11: close the refill form
                context.events.onReFillForm(false);
                break;
            case 12: { // 35/12: u8 account password change result (north-island-server only)
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int result = reader.u8();
                if (!reader.failed()) {
                    context.events.onAccountPasswordChanged(result);
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            case 7: { // 35/7: u8 recommend result
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int result = reader.u8();
                if (!reader.failed() && (result == 1 || result == 2)) { // the two known results
                    context.events.onRecommendResult(result);
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            case 9: { // 35/9: i32 + i32 recommend points
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const std::int32_t points = reader.i32();
                const std::int32_t more = reader.i32();
                if (!reader.failed()) {
                    context.events.onRecommendPoints(points + more);
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            case 10: { // 35/10: u8 kind: server busy notice
                core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
                const int kind = reader.u8();
                if (!reader.failed()) {
                    if (kind == 1) { // kind 1: server busy
                        context.events.onNoticeMessage("The server is busy now. Try later.", 2000); // notice display time in ms
                    }
                    break;
                }
                context.events.onUnhandled(id);
                break;
            }
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
