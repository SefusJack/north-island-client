#include "protocol/StatusEffectRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes timeEffectRequest(int sub) {
        core::ByteWriter writer;
        writer.u8(5).u8(static_cast<std::uint8_t>(sub)); // 5 = time effect action
        return writer.take();
    }

    core::Bytes trainingIslandRequest(int op) {
        core::ByteWriter writer;
        writer.u8(23).u8(111).u8(static_cast<std::uint8_t>(op)); // 23 = item action, 111 = Training Island sub id
        return writer.take();
    }

    core::Bytes strengthBuffRequest() {
        core::ByteWriter writer;
        writer.u8(23).u8(119).u8(2); // 119 = stat buff sub id, 2 = ask minutes left
        return writer.take();
    }

    core::Bytes strengthBuffUseRequest(int bagSlot) {
        core::ByteWriter writer;
        // 119 = stat buff sub id, 3 = use item
        writer.u8(23).u8(119).u8(3).u8(static_cast<std::uint8_t>(bagSlot));
        return writer.take();
    }

    core::Bytes bodyShiftToggleRequest(bool on) {
        core::ByteWriter writer;
        writer.u8(23).u8(114).u8(on ? 3 : 4); // 114 = body shift sub id, 3 = switch on, 4 = switch off
        return writer.take();
    }

    core::Bytes bodyShiftUseRequest(int bagSlot) {
        core::ByteWriter writer;
        // 114 = body shift sub id, 1 = use Riceball
        writer.u8(23).u8(114).u8(1).u8(static_cast<std::uint8_t>(bagSlot));
        return writer.take();
    }

    core::Bytes bodyShiftAskRequest() {
        core::ByteWriter writer;
        writer.u8(23).u8(114).u8(2); // 114 = body shift sub id, 2 = ask time left
        return writer.take();
    }

}
