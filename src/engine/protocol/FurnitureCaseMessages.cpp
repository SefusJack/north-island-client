#include "protocol/FurnitureCaseMessages.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kHouse = 62; // action 62: house / furniture
        constexpr std::uint8_t kCase = 58; // 62/58: showcase owner messages (option byte follows)
        constexpr std::uint8_t kBuyFromCase = 59; // 62/59: buy from a showcase
        constexpr std::size_t kMessageHead = 3; // bytes before the body: action, sub, option

        /// @brief Starts a client 62/58 message with its option byte.
        /// @param option The option (1 open, 2 add, 3 take back, 4 reprice).
        /// @return A writer holding the message head.
        core::ByteWriter caseMessage(std::uint8_t option) {
            core::ByteWriter writer;
            writer.u8(kHouse).u8(kCase).u8(option);
            return writer;
        }

    }

    std::optional<FurnitureCaseUpdate> decodeFurnitureCaseUpdate(const core::Bytes& payload) {
        core::ByteReader reader(payload, kMessageHead);
        FurnitureCaseUpdate update;
        update.ownerId = reader.i32();
        update.caseNo = reader.u8();
        update.furnIndex = reader.u8();
        update.posInFurn = reader.u8();
        update.itemId = reader.u16();
        update.count = reader.u8();
        update.price = reader.i32();
        if (reader.failed()) {
            return std::nullopt;
        }
        return update;
    }

    std::optional<FurnitureCaseSale> decodeFurnitureCaseSale(const core::Bytes& payload) {
        core::ByteReader reader(payload, kMessageHead);
        FurnitureCaseSale sale;
        sale.ownerId = reader.i32();
        sale.caseNo = reader.u16();
        sale.count = reader.u8();
        sale.furnIndex = reader.u16();
        if (reader.failed()) {
            return std::nullopt;
        }
        return sale;
    }

    core::Bytes openCaseRequest(int piece, int bagSlot, std::int32_t price) {
        core::ByteWriter writer = caseMessage(1);
        writer.u8(static_cast<std::uint8_t>(piece)).u8(static_cast<std::uint8_t>(bagSlot)).i32(price);
        return writer.take();
    }

    core::Bytes addToCaseRequest(int piece, int bagSlot, int caseNo, int room) {
        core::ByteWriter writer = caseMessage(2); // 62/58 option 2: add to a case
        writer.u8(static_cast<std::uint8_t>(piece)).u8(static_cast<std::uint8_t>(bagSlot)).u8(static_cast<std::uint8_t>(caseNo));
        if (room != 0) {
            writer.u8(static_cast<std::uint8_t>(room));
        }
        return writer.take();
    }

    core::Bytes takeFromCaseRequest(int piece, int caseNo, int count) {
        core::ByteWriter writer = caseMessage(3); // 62/58 option 3: take back
        writer.u8(static_cast<std::uint8_t>(piece)).u8(static_cast<std::uint8_t>(caseNo)).u8(static_cast<std::uint8_t>(count));
        return writer.take();
    }

    core::Bytes repriceCaseRequest(int piece, int caseNo, std::int32_t price) {
        core::ByteWriter writer = caseMessage(4); // 62/58 option 4: reprice
        writer.u8(static_cast<std::uint8_t>(piece)).u8(static_cast<std::uint8_t>(caseNo)).i32(price);
        return writer.take();
    }

    core::Bytes buyFromCaseRequest(std::int32_t ownerId, int caseNo, int count, std::int32_t price) {
        core::ByteWriter writer;
        writer.u8(kHouse).u8(kBuyFromCase).i32(ownerId).u16(static_cast<std::uint16_t>(caseNo)).u8(static_cast<std::uint8_t>(count)).i32(price);
        return writer.take();
    }

}
