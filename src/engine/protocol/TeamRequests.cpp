#include "protocol/TeamRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kTeam = 13; // action 13: team

        /// @brief Builds a team request of the shape sub id plus i32 id.
        /// @param sub Team sub id.
        /// @param id Player id argument.
        /// @return The encoded request payload.
        core::Bytes withId(std::uint8_t sub, std::int32_t id) {
            core::ByteWriter writer;
            writer.u8(kTeam).u8(sub).i32(id);
            return writer.take();
        }

        /// @brief Builds a team request of the shape sub id, u8 answer, i32 id.
        /// @param sub Team sub id.
        /// @param answer Answer byte.
        /// @param id Player id argument.
        /// @return The encoded request payload.
        core::Bytes withAnswer(std::uint8_t sub, int answer, std::int32_t id) {
            core::ByteWriter writer;
            writer.u8(kTeam).u8(sub).u8(static_cast<std::uint8_t>(answer)).i32(id);
            return writer.take();
        }

    }

    core::Bytes joinTeamRequest(std::int32_t targetId) { return withId(1, targetId); } // 1 = join team sub id

    core::Bytes joinAnswerRequest(int answer, std::int32_t requesterId) { return withAnswer(3, answer, requesterId); } // 3 = join answer sub id

    core::Bytes leaveTeamRequest(std::int32_t leaderId) { return withId(4, leaderId); } // 4 = leave team sub id

    core::Bytes inviteRequest(std::int32_t targetId) { return withId(7, targetId); } // 7 = invite sub id

    core::Bytes inviteAnswerRequest(int answer, std::int32_t inviterId) { return withAnswer(8, answer, inviterId); } // 8 = invite answer sub id

    core::Bytes kickMemberRequest(std::int32_t memberId) { return withId(9, memberId); } // 9 = kick member sub id

    core::Bytes offerLeadRequest(std::int32_t memberId) { return withAnswer(10, 1, memberId); } // 10 = leader change sub id, 1 = offer to lead

    core::Bytes leaderAnswerRequest(int answer) {
        return {kTeam, 10, 2, static_cast<std::uint8_t>(answer)};
    } // 10 = leader change sub id, 2 = answer to the offer

    core::Bytes leaderAcceptRequest() { return {kTeam, 10, 3}; } // 10 = leader change sub id, 3 = offer accepted

}
