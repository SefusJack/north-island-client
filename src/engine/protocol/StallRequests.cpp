#include "protocol/StallRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kItems = 23; // action 23: items

    }

    core::Bytes openItemStallRequest(const std::string& name, int sign, const std::vector<OwnStall::Price>& prices) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(30).shortText(name).u8(static_cast<std::uint8_t>(sign)); // 30 = open item stall sub id
        for (const OwnStall::Price& price : prices) {
            writer.u8(static_cast<std::uint8_t>(price.bagSlot)).i32(price.price);
        }
        return writer.take();
    }

    core::Bytes closeItemStallRequest() {
        core::ByteWriter writer;
        writer.u8(kItems).u8(31); // 31 = close item stall sub id
        return writer.take();
    }

    core::Bytes browseItemStallRequest(std::int32_t ownerId) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(32).i32(ownerId); // 32 = browse item stall sub id
        return writer.take();
    }

    core::Bytes openPetStallRequest(const std::string& name, const std::vector<OwnStall::Price>& prices) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(60).shortText(name).u8(static_cast<std::uint8_t>(prices.size())); // 60 = open pet stall sub id
        for (const OwnStall::Price& price : prices) {
            writer.u8(static_cast<std::uint8_t>(price.bagSlot)).i32(price.price);
        }
        return writer.take();
    }

    core::Bytes closePetStallRequest() {
        core::ByteWriter writer;
        writer.u8(kItems).u8(61); // 61 = close pet stall sub id
        return writer.take();
    }

    core::Bytes browsePetStallRequest(std::int32_t ownerId) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(62).i32(ownerId); // 62 = browse pet stall sub id
        return writer.take();
    }

    core::Bytes buyStallPetsRequest(std::int32_t ownerId, const std::vector<int>& places) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(63).i32(ownerId); // 63 = buy stall pets sub id
        for (const int place : places) {
            writer.u8(static_cast<std::uint8_t>(place));
        }
        return writer.take();
    }

    core::Bytes buyFromStallRequest(std::int32_t ownerId, const std::vector<StallPurchase>& lines) {
        core::ByteWriter writer;
        writer.u8(kItems).u8(33).i32(ownerId); // 33 = buy from item stall sub id
        for (const StallPurchase& line : lines) {
            writer.u8(static_cast<std::uint8_t>(line.stallSlot))
                .u8(static_cast<std::uint8_t>(line.count))
                .u16(static_cast<std::uint16_t>(line.itemId))
                .u32(line.echoKey);
        }
        return writer.take();
    }

}
