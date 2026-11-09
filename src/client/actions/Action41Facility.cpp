#include "client/actions/Action41Facility.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <utility>
#include <vector>

namespace client {

    void handleAction41Facility(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        if (id.sub == 1) { // 41/1: compound facility update (RecvCompoundFacilityUpdate)
            core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
            const int mode = reader.u8(); // sub mode: 1 product + facility pairs, 2 products only
            const int count = reader.u8();
            std::vector<std::pair<int, int>> changes;
            for (int index = 0; index < count && !reader.failed(); ++index) {
                const int product = reader.u16();
                const int facility = mode == 1 ? reader.u16() : 0; // mode 1 carries a facility id
                if (!reader.failed()) {
                    changes.emplace_back(product, facility);
                }
            }
            if (mode == 1 || mode == 2) { // the two known modes
                context.events.onCompoundFacility(changes);
            }
            return;
        }
        context.events.onUnhandled(id);
    }

}
