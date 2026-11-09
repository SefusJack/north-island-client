#include "protocol/AccountRequests.h"

#include "protocol/MessageId.h"

#include <cstdint>

namespace protocol {

    core::Bytes selectCharacterRequest(int slot) { return {kActionAccount, 2, static_cast<std::uint8_t>(slot)}; } // 63/2: select slot

    core::Bytes leaveCharacterSelectRequest() { return {kActionAccount, 0}; }

    core::Bytes cancelCreateCharacterRequest() { return {kActionAccount, 3}; } // 63/3: leave character creation

    core::Bytes checkNameRequest(const std::string& name) {
        core::Bytes message{kActionCreateCharacter, 2}; // 9/2: name check
        message.insert(message.end(), name.begin(), name.end());
        return message;
    }

}
