#include "protocol/ChangePasswordRequest.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes ChangePasswordRequest::encode() const {
        core::ByteWriter writer;
        writer.u8(35).u8(1).shortText(oldPassword).shortText(newPassword); // 35/1: change the secret code (action 35)
        return writer.take();
    }

    core::Bytes ChangeAccountPasswordRequest::encode() const {
        core::ByteWriter writer;
        writer.u8(35).u8(12).shortText(oldPassword).shortText(newPassword); // 35/12: change the account password (action 35)
        return writer.take();
    }

}
