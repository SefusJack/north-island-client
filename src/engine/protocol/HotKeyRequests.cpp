#include "protocol/HotKeyRequests.h"

#include "core/ByteReader.h"
#include "core/ByteWriter.h"

namespace protocol {

    std::vector<HotKeyRecord> decodeHotKeys(const core::Bytes& payload) {
        constexpr std::size_t kRecord = 5;
        std::vector<HotKeyRecord> records;
        core::ByteReader reader(payload, 2); // skip the action and sub bytes
        while (reader.remaining() >= kRecord) {
            HotKeyRecord record;
            record.kind = reader.u8();
            record.id = reader.u16();
            record.page = reader.u8();
            record.slot = reader.u8();
            records.push_back(record);
        }
        return records;
    }

    core::Bytes hotKeyRequest(const HotKeyRecord& record) {
        core::ByteWriter writer;
        // 40/1: hot key place
        writer.u8(40).u8(1).u8(static_cast<std::uint8_t>(record.kind)).u16(static_cast<std::uint16_t>(record.id));
        writer.u8(static_cast<std::uint8_t>(record.page)).u8(static_cast<std::uint8_t>(record.slot));
        return writer.take();
    }

    core::Bytes hotKeyUseRequest(
        int kind,
        int id,
        int casterPetSlot,
        int targetKind,
        int targetSlot,
        std::int32_t targetId
    ) {
        core::ByteWriter writer;
        // 23/20: use a bar item or healing skill
        writer.u8(23).u8(20).u8(static_cast<std::uint8_t>(kind)).u16(static_cast<std::uint16_t>(id));
        writer.u8(static_cast<std::uint8_t>(casterPetSlot)).u8(static_cast<std::uint8_t>(targetKind)).u8(static_cast<std::uint8_t>(targetSlot));
        writer.i32(targetId);
        return writer.take();
    }

}
