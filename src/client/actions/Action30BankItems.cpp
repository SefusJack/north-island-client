#include "client/actions/Action30BankItems.h"

#include "core/ByteReader.h"
#include "protocol/BankLists.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction30BankItems(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: // 30/1: the bank's items in their places
                for (const protocol::BagItemRecord& record : protocol::decodeBankItems(payload)) {
                    context.events.onBankItemPut(record);
                }
                break;
            case 2: // 30/2: an item the client places
                if (const auto item = protocol::decodeBankItem(payload)) {
                    context.events.onBankItemAdded(*item);
                } else {
                    context.events.onUnhandled(id);
                }
                break;
            case 3: { // 30/3: u16 item, u8 count: taken out
                const int itemId = reader.u16();
                const int count = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onBankItemRemoved(itemId, count);
                break;
            }
            case 4: // 30/4: an item in a place
            case 10: { // 30/10: an item at a given slot
                const auto record =
                    id.sub == 4 ? protocol::decodeBankItemPut(payload) : protocol::decodeBankItemAtSlot(payload); // sub 4 = put record, else record with slot
                if (record) {
                    context.events.onBankItemPut(*record);
                } else {
                    context.events.onUnhandled(id);
                }
                break;
            }
            case 5: { // 30/5: u8 place, u8 count: fewer in a place
                const int slot = reader.u8();
                const int count = reader.u8();
                if (reader.failed()) {
                    context.events.onUnhandled(id);
                    return;
                }
                context.events.onBankSlotDecreased(slot, count);
                break;
            }
            case 6: // 30/6: a withdrawal is done
            case 7: // 30/7: a deposit is done
                context.events.onBankItemsDone(id.sub == 7); // sub 7 = deposit
                break;
            case 8: // 30/8: the bank is emptied
                context.events.onBankCleared();
                break;
            default: // any other sub id: reported as unhandled
                context.events.onUnhandled(id);
                break;
        }
    }

}
