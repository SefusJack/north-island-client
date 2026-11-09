#include "protocol/ServerSelected.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"

namespace protocol {

    std::optional<ServerSelected> ServerSelected::decode(const core::Bytes& payload) {
        core::ByteReader reader(payload);
        if (reader.u8() != kActionSystem || reader.u8() != kSystemServerSelected) {
            return std::nullopt;
        }
        ServerSelected message;
        message.serverId = reader.u16();
        message.grouped = reader.u8() == 1;
        message.serverName = reader.rest();
        if (reader.failed()) {
            return std::nullopt;
        }
        return message;
    }

}
