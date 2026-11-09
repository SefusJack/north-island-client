#include "protocol/InstanceRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kInstances = 85; // action 85: instances (dungeons)

        /// @brief Starts an instance (action 85) packet with its sub code.
        /// @param sub Sub code.
        /// @return A writer holding action and sub.
        core::ByteWriter start(std::uint8_t sub) {
            core::ByteWriter writer;
            writer.u8(kInstances).u8(sub);
            return writer;
        }

    }

    core::Bytes instanceListRequest() { return start(1).take(); } // sub 1: list of open instances

    core::Bytes instancePageRequest(int page) { return start(2).u8(static_cast<std::uint8_t>(page)).take(); } // sub 2: page of the list

    core::Bytes instanceCreateRequest(int sceneId, const std::string& message) {
        return start(3).u16(static_cast<std::uint16_t>(sceneId)).shortText(message).take(); // sub 3: create an instance
    }

    core::Bytes instanceDetailRequest(int mapId) { return start(4).u16(static_cast<std::uint16_t>(mapId)).take(); } // sub 4: detail window

    core::Bytes instanceJoinRequest(int mapId) { return start(5).u16(static_cast<std::uint16_t>(mapId)).take(); } // sub 5: join the instance

    core::Bytes instanceLeaveRequest() { return start(6).take(); } // sub 6: leave the instance

    core::Bytes instanceEnterRequest() { return start(9).take(); } // sub 9: go into the instance

    core::Bytes instanceMembersRequest() { return start(10).take(); } // sub 10: members tab opened

    core::Bytes instanceMembersPageRequest(int page) { return start(11).u8(static_cast<std::uint8_t>(page)).take(); } // sub 11: page of the members

    core::Bytes instanceKickRequest(int mapId, std::int32_t memberId) {
        return start(13).u16(static_cast<std::uint16_t>(mapId)).i32(memberId).take(); // sub 13: kick a member
    }

}
