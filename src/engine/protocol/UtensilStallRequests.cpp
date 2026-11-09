#include "protocol/UtensilStallRequests.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items
        constexpr std::uint8_t kOpen = 65; // 23/65: open the utensil stall
        constexpr std::uint8_t kClose = 66; // 23/66: close the utensil stall
        constexpr std::uint8_t kBrowse = 67; // 23/67: look at a utensil stall
        constexpr std::uint8_t kBuy = 68; // 23/68: buy from a utensil stall
        constexpr std::size_t kSkipped = 12; // bytes skipped after the owner id, before the piece records
        constexpr std::size_t kPieceRecord = 15; // bytes per piece record: u8 place, u16 item, u8 count, u8 wear, i32 price, i32 used, u16 extra
        constexpr std::size_t kMostPieces = 3;

    }

    core::Bytes openUtensilStallRequest(const std::string& name, const std::vector<OwnStall::Price>& prices) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kOpen).shortText(name).u8(static_cast<std::uint8_t>(prices.size()));
        for (const OwnStall::Price& price : prices) {
            writer.u8(static_cast<std::uint8_t>(price.bagSlot)).i32(price.price);
        }
        return writer.take();
    }

    core::Bytes closeUtensilStallRequest() {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kClose);
        return writer.take();
    }

    core::Bytes browseUtensilStallRequest(std::int32_t ownerId) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kBrowse).i32(ownerId);
        return writer.take();
    }

    core::Bytes buyUtensilsRequest(std::int32_t ownerId, const std::vector<UtensilPurchase>& lines) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(kBuy).i32(ownerId);
        for (const UtensilPurchase& line : lines) {
            writer.u8(static_cast<std::uint8_t>(line.listPlace)).u8(static_cast<std::uint8_t>(line.count));
        }
        return writer.take();
    }

    std::optional<UtensilStallGoods> decodeUtensilStallGoods(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2);
        UtensilStallGoods goods;
        goods.ownerId = reader.i32();
        reader.skip(kSkipped);
        if (reader.failed()) {
            return std::nullopt;
        }
        while (reader.remaining() >= kPieceRecord && goods.pieces.size() < kMostPieces) {
            UtensilStallGoods::Piece piece;
            piece.listPlace = reader.u8();
            piece.item.itemId = reader.u16();
            piece.item.count = reader.u8();
            piece.item.wear = reader.u8();
            piece.price = reader.i32();
            piece.item.usedAmount = reader.i32();
            piece.item.extraAmount = reader.u16();
            goods.pieces.push_back(piece);
        }
        return goods;
    }

}
