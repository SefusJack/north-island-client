#include "protocol/ShopRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kShop = 27; // action 27: shop
        constexpr std::uint8_t kInn = 31; // action 31: inn

    }

    core::Bytes npcStoreBuyRequest(const std::vector<std::pair<int, int>>& rowsAndCounts) {
        core::ByteWriter writer;
        writer.u8(kShop).u8(1).u8(static_cast<std::uint8_t>(rowsAndCounts.size())); // 1 = buy sub id
        for (const auto& [row, count] : rowsAndCounts) {
            writer.u8(static_cast<std::uint8_t>(row)).u8(static_cast<std::uint8_t>(count));
        }
        return writer.take();
    }

    core::Bytes npcSaleRequest(const std::vector<int>& bagSlots, int saleKind) {
        core::ByteWriter writer;
        writer.u8(kShop).u8(2); // 2 = sell sub id
        for (const int slot : bagSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        writer.u8(static_cast<std::uint8_t>(saleKind));
        return writer.take();
    }

    core::Bytes innRestRequest() { return {31, 1}; } // 31 = inn action, 1 = rest sub id

    core::Bytes petStorageTakeRequest(const std::vector<int>& places) {
        core::ByteWriter writer;
        writer.u8(kInn).u8(2); // 2 = take out sub id
        for (const int place : places) {
            writer.u8(static_cast<std::uint8_t>(place));
        }
        return writer.take();
    }

    core::Bytes petStorageStoreRequest(const std::vector<int>& petSlots) {
        core::ByteWriter writer;
        writer.u8(kInn).u8(3); // 3 = store sub id
        for (const int slot : petSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        return writer.take();
    }

    core::Bytes petStorageSwapRequest(const std::vector<int>& petSlots, const std::vector<int>& places) {
        core::ByteWriter writer;
        writer.u8(kInn).u8(4).u8(static_cast<std::uint8_t>(petSlots.size())); // 4 = swap sub id
        for (const int slot : petSlots) {
            writer.u8(static_cast<std::uint8_t>(slot));
        }
        writer.u8(static_cast<std::uint8_t>(places.size()));
        for (const int place : places) {
            writer.u8(static_cast<std::uint8_t>(place));
        }
        return writer.take();
    }

}
