#include "protocol/IdAndPwRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes idAndPwRequest(int sub, const std::string& account, const std::string& password) {
        constexpr std::uint8_t kIdAndPwAction = 21; // action 21: account id and password
        core::ByteWriter writer;
        writer.u8(kIdAndPwAction).u8(static_cast<std::uint8_t>(sub)).u8(static_cast<std::uint8_t>(account.size()));
        for (const char byte : account) {
            writer.u8(static_cast<std::uint8_t>(byte));
        }
        writer.u8(static_cast<std::uint8_t>(password.size()));
        for (const char byte : password) {
            writer.u8(static_cast<std::uint8_t>(byte));
        }
        return writer.take();
    }

}
