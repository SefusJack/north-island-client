#include "protocol/ChangePasswordRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes ChangePasswordRequest::encode() const {
        core::ByteWriter writer;
        writer.u8(35).u8(1).shortText(oldPassword).shortText(newPassword); // 35/1: change password (action 35)
        return writer.take();
    }

}
