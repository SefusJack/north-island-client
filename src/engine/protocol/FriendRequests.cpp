#include "protocol/FriendRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kFriends = 14; // action 14: friends and mail

    }

    core::Bytes mailRequest(const std::vector<std::int32_t>& receivers, const std::string& text) {
        core::ByteWriter writer;
        writer.u8(kFriends).u8(1).u8(static_cast<std::uint8_t>(receivers.size())); // 14/1: send a mail to the players
        for (std::int32_t id : receivers) {
            writer.i32(id);
        }
        writer.text(text);
        return writer.take();
    }

    core::Bytes furMailRequest(std::int32_t ownId, const std::string& name, const std::string& text) {
        core::ByteWriter writer;
        writer.u8(kFriends).u8(6).i32(ownId).shortText(name).text(text); // 14/6: mail by name
        return writer.take();
    }

    core::Bytes addFriendRequest(std::int32_t playerId) {
        core::ByteWriter writer;
        writer.u8(kFriends).u8(2).i32(playerId); // 14/2: ask to be friends
        return writer.take();
    }

    core::Bytes friendAnswerRequest(std::int32_t playerId, int answer) {
        core::ByteWriter writer;
        writer.u8(kFriends).u8(3).i32(playerId).u8(static_cast<std::uint8_t>(answer)); // 14/3: answer a friend request
        return writer.take();
    }

    core::Bytes deleteFriendRequest(std::int32_t friendId) {
        core::ByteWriter writer;
        writer.u8(kFriends).u8(4).i32(friendId); // 14/4: delete a friend
        return writer.take();
    }

    core::Bytes blacklistRequest(int change, std::int32_t playerId, const std::string& name) {
        core::ByteWriter writer;
        writer.u8(14).u8(5).u8(static_cast<std::uint8_t>(change)).i32(playerId).shortText(name); // 14/5: black list change (14 is the friends action)
        return writer.take();
    }

}
