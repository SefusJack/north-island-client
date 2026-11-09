#include "protocol/ReFillCardRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes ReFillCardRequest::encode() const {
        core::ByteWriter writer;
        writer.u8(35).u8(3).shortText(cardNumber).shortText(password).u8(0); // action 35, sub 3: recharge with a prepaid card
        return writer.take();
    }

}
