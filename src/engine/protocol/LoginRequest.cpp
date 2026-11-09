#include "protocol/LoginRequest.h"

#include "core/ByteWriter.h"
#include "protocol/MessageId.h"

namespace protocol {

    core::Bytes LoginRequest::encode() const {
        core::ByteWriter writer;
        // sub 4: log in
        writer.u8(kActionAccount).u8(4).shortText(account).shortText(password).u16(clientVersion);
        if (!itemDatSize) {
            writer.u8(0).u8(key);
            return writer.take();
        }
        const std::string digits = std::to_string(*itemDatSize);
        writer.u8(static_cast<std::uint8_t>(digits.size())).u8(key);
        for (char digit : digits) {
            writer.u8(static_cast<std::uint8_t>(static_cast<std::uint8_t>(digit) ^ key));
        }
        return writer.take();
    }

}
