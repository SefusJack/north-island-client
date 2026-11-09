#include "client/actions/Action12MapChange.h"

#include "core/ByteReader.h"

#include <array>

namespace client {

    void handleAction12MapChange(const core::Bytes& payload, MessageContext& context) {
        core::ByteReader reader(payload, 1);
        const std::int32_t player = reader.i32();
        const int mapId = reader.u16();
        const int x = reader.u16();
        const int y = reader.u16();
        reader.u16();
        const int houseRoom = reader.u8();
        if (reader.failed()) {
            context.events.onMapChanged(player, mapId, core::Point{x, y});
            return;
        }
        std::array<int, 5> sizes{}; // 5 floors of room sizes
        bool sizesGiven = false;
        constexpr int kMostPairs = 6;
        for (int pair = 0; reader.remaining() >= 2 && pair < kMostPairs; ++pair) { // a pair is 2 bytes (size, floor)
            const int size = reader.u8();
            const int floor = reader.u8();
            if (floor >= 0 && floor < static_cast<int>(sizes.size())) {
                sizes[static_cast<std::size_t>(floor)] = size;
            }
            sizesGiven = true;
        }
        context.events.onHouseRoom(player, houseRoom, sizesGiven, sizes);
        context.events.onMapChanged(player, mapId, core::Point{x, y});
    }

}
