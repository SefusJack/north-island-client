#include "client/actions/Action24Marks.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction24Marks(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 1: // 24/1: u16 mark, u8 count: marks added
            case 2: { // 24/2: same fields: marks taken off
                const int markId = reader.u16();
                const int count = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMarkCount(markId, id.sub == 1 ? count : -count);
                return;
            }
            case 3: // 24/3: the quest volume is full
                context.events.onQuestsFull();
                return;
            case 4: { // 24/4: u16 mark deleted
                const int markId = reader.u16();
                if (reader.failed()) {
                    break;
                }
                context.events.onMarkDeleted(markId);
                return;
            }
            case 5: { // 24/5: u16 flag bit, u8 value (0 / 1)
                const int flagBit = reader.u16();
                const int value = reader.u8();
                if (reader.failed()) {
                    break;
                }
                context.events.onMarkFlag(flagBit, value);
                return;
            }
            case 6: { // 24/6: (u8 place, u16 mark, u8 count) records
                constexpr std::size_t kRecord = 4;
                const std::size_t count = (payload.size() - 2) / kRecord; // 2: header size
                for (std::size_t i = 0; i < count; ++i) {
                    const int slot = reader.u8();
                    const int markId = reader.u16();
                    const int markCount = reader.u8();
                    context.events.onMarkSlot(slot, markId, markCount);
                }
                return;
            }
            case 7: { // 24/7: (u8 byte index, u8 bits) pairs
                const std::size_t count = (payload.size() - 2) / 2; // 2: header size; 2: bytes per pair
                for (std::size_t i = 0; i < count; ++i) {
                    const int byteIndex = reader.u8();
                    const int bits = reader.u8();
                    context.events.onMarkFlagByte(byteIndex, bits);
                }
                return;
            }
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
