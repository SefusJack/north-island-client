#include "protocol/BlackBoardRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    core::Bytes blackBoardRequest(int sub, std::int32_t houseOwnerId, const std::string& data) {
        constexpr int kBlackBoardAction = 72; // action 72: black board (house)
        core::ByteWriter writer;
        writer.u8(static_cast<std::uint8_t>(kBlackBoardAction)).u8(static_cast<std::uint8_t>(sub)).i32(houseOwnerId);
        for (const char byte : data) {
            writer.u8(static_cast<std::uint8_t>(byte));
        }
        return writer.take();
    }

}
