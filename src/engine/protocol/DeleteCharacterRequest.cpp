#include "protocol/DeleteCharacterRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes DeleteCharacterRequest::encode() const {
        core::ByteWriter writer;
        writer.u8(35).u8(2).u8(static_cast<std::uint8_t>(slot)).shortText(password).shortText(cipher); // 35/2: delete character (action 35)
        return writer.take();
    }

}
