#ifndef NORTH_ISLAND_PROTOCOL_MESSAGE_ID_H
#define NORTH_ISLAND_PROTOCOL_MESSAGE_ID_H

#include "core/Bytes.h"

#include <cstdint>

namespace protocol {

    /// @brief The first two payload bytes: the action and, for most actions, a sub code.
    struct MessageId {
        std::uint8_t action = 0;
        std::uint8_t sub = 0;

        /// @brief Reads the action and sub code from a payload.
        /// @param payload The packet payload; missing bytes read as 0.
        /// @return The message id.
        static MessageId of(const core::Bytes& payload) {
            MessageId id;
            if (!payload.empty()) {
                id.action = payload[0];
            }
            if (payload.size() > 1) {
                id.sub = payload[1];
            }
            return id;
        }

        /// @brief Tells whether this id has the given action and sub code.
        /// @param a Action.
        /// @param s Sub code.
        /// @return True on a match.
        bool is(std::uint8_t a, std::uint8_t s) const { return action == a && sub == s; }
    };

    constexpr std::uint8_t kActionHello = 0; // action 0: server kick / the client's reply to it
    constexpr std::uint8_t kActionSystem = 1; // action 1: system results (login errors, server selection)
    constexpr std::uint8_t kActionCreateCharacter = 9; // action 9: character creation
    constexpr std::uint8_t kActionAccount = 63; // action 63: account login

    constexpr std::uint8_t kSystemTimeout = 5; // action 1 sub 5: timeout
    constexpr std::uint8_t kSystemWrongPassword = 6; // action 1 sub 6: wrong account or password
    constexpr std::uint8_t kSystemConnectionFailed = 7; // action 1 sub 7: connection failed
    constexpr std::uint8_t kSystemCharacterLocked = 8; // action 1 sub 8: character locked
    constexpr std::uint8_t kSystemServerSelected = 9; // action 1 sub 9: server selected
    constexpr std::uint8_t kSystemAccountLocked = 10; // action 1 sub 10: account locked

}

#endif
