#include "client/actions/Action83PetVoucher.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    namespace {

        constexpr int kShortMs = 1500; // notice display time, ms

    }

    void handleAction83PetVoucher(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 83/1: pet slots not enough to open a pet egg
                context.events.onNoticeMessage("Pet slot is not enough, can't open pet egg", kShortMs);
                return;
            case 2: // 83/2: already has the same pet
                context.events.onNoticeMessage("Has the same pet", kShortMs);
                return;
            case 3: // 83/3: same pet in the tent
                context.events.onNoticeMessage("Has the same pet in the tent", kShortMs);
                return;
            case 4: // 83/4: pet keeper has the same pet
                context.events.onNoticeMessage("Pet keeper has the same pet", kShortMs);
                return;
            case 5: // 83/5: pet cases not enough for the exchange voucher
                context.events.onNoticeMessage("Pet cases are not enough, you can't use pet exhange voucher", kShortMs);
                return;
            case 6: { // 83/6: u16 item the voucher gave
                core::ByteReader reader(payload, 2); // skip the action and sub id bytes
                const int itemId = reader.u16();
                if (!reader.failed()) {
                    context.events.onVoucherItem(itemId);
                    return;
                }
                break;
            }
            default: // unknown sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
