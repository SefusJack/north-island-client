#include "client/actions/Action51FighterAttributes.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace client {

    void handleAction51FighterAttributes(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 2) { // 51/2: battle timer reset, no data
            context.events.onBattleTimeReset();
            return;
        }
        if (id.sub != 1) { // 51/1: attribute records, the only other sub id
            context.events.onUnhandled(id);
            return;
        }
        constexpr std::size_t kRecord = 7;
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        while (reader.remaining() >= kRecord) {
            const int row = reader.u8();
            const int col = reader.u8();
            const int attribute = reader.u8();
            const std::int32_t value = reader.i32();
            context.events.onFighterAttribute(row, col, attribute, value);
        }
    }

}
