#include "protocol/MarriageRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kMarriage = 45; // action 45: marriage

    }

    core::Bytes spouseCardRequest() {
        core::ByteWriter writer;
        writer.u8(kMarriage).u8(4); // sub 4: spouse card
        return writer.take();
    }

    core::Bytes spouseMailRequest(std::int32_t ownId, const std::string& spouseName, const std::string& text) {
        core::ByteWriter writer;
        // sub 6: mail to the spouse
        writer.u8(kMarriage).u8(6).i32(ownId).shortText(spouseName).text(text);
        return writer.take();
    }

    core::Bytes petShownRequest(std::int32_t ownId) {
        core::ByteWriter writer;
        writer.u8(kMarriage).u8(8).i32(ownId); // sub 8: pet shown again
        return writer.take();
    }

    core::Bytes marryAnswerRequest(std::int32_t ownId, std::int32_t partnerId, bool accept) {
        core::ByteWriter writer;
        writer.u8(kMarriage).u8(11).i32(ownId).i32(partnerId).u8(accept ? 1 : 0); // sub 11: marriage application answer
        return writer.take();
    }

    core::Bytes divorceAnswerRequest(std::int32_t ownId, std::int32_t partnerId, bool accept) {
        core::ByteWriter writer;
        writer.u8(kMarriage).u8(12).i32(ownId).i32(partnerId).u8(accept ? 1 : 0); // sub 12: divorce application answer
        return writer.take();
    }

    core::Bytes divorceConsentRequest(bool agree) {
        core::ByteWriter writer;
        writer.u8(kMarriage).u8(9).u8(agree ? 1 : 0); // sub 9: spouse's divorce request answer
        return writer.take();
    }

}
