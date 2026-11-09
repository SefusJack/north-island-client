#include "protocol/ItemBagRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItemBag = 68; // action 68: Item Bag (5 x 5 second bag)

    }

    core::Bytes itemBagMoveRequest(ItemBagMove kind, int from, int to, int count) {
        core::ByteWriter writer;
        writer.u8(kItemBag)
            .u8(static_cast<std::uint8_t>(kind))
            .u8(static_cast<std::uint8_t>(from))
            .u8(static_cast<std::uint8_t>(to))
            .u8(static_cast<std::uint8_t>(count));
        return writer.take();
    }

}
