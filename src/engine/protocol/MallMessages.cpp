#include "protocol/MallMessages.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    std::vector<MallRow> decodeMallCatalog(const core::Bytes& payload) {
        std::vector<MallRow> rows;
        core::ByteReader reader(payload, 2); // skip action and sub
        const int count = reader.u16();
        for (int i = 0; i < count && !reader.failed(); ++i) {
            MallRow row;
            row.itemId = reader.u16();
            row.count = reader.u8();
            row.price = reader.u16();
            row.discount = reader.u8();
            row.hot = reader.u8();
            row.category = reader.u8();
            row.key = reader.u16();
            if (!reader.failed()) {
                rows.push_back(row);
            }
        }
        return rows;
    }

    core::Bytes mallBuyRequest(const std::vector<MallPurchaseLine>& lines) {
        core::ByteWriter writer;
        writer.u8(75).u8(1).u8(static_cast<std::uint8_t>(lines.size())); // action 75 (item mall), sub 1: buy the cart
        for (const MallPurchaseLine& line : lines) {
            writer.u16(static_cast<std::uint16_t>(line.itemId)).u8(static_cast<std::uint8_t>(line.category));
            writer.u8(static_cast<std::uint8_t>(line.count)).u16(static_cast<std::uint16_t>(line.key));
        }
        return writer.take();
    }

    core::Bytes mallCatalogRequest() { return {75, 2}; } // action 75, sub 2: catalog again

    core::Bytes mallForgeRequest(int bagSlot) { return {75, 3, static_cast<std::uint8_t>(bagSlot)}; } // action 75, sub 3: forge

    core::Bytes mallGameRequest(int kind) { return {75, 4, static_cast<std::uint8_t>(kind)}; } // action 75, sub 4: games tab slot machine

    core::Bytes imPointsRequest(int forPayment) { return {34, 1, static_cast<std::uint8_t>(forPayment)}; } // action 34, sub 1: IM points

}
