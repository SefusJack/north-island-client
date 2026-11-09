#ifndef NORTH_ISLAND_PROTOCOL_PROFILE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_PROFILE_REQUESTS_H

#include "core/Bytes.h"

#include <string>

namespace protocol {

    /// @brief Builds 5/7: set the state friends see.
    /// @param status 0 online, 1 busy (u8).
    /// @return The encoded payload.
    core::Bytes onlineStatusRequest(int status);

    /// @brief Builds 10/1: set the nickname (pstr8).
    /// @param nickname The nickname.
    /// @return The encoded payload.
    core::Bytes nicknameRequest(const std::string& nickname);

    /// @brief Builds 10/2: set the profile.
    /// @param heartNumber Heart number (u8).
    /// @param bloodType Blood type (u8).
    /// @param birthYear Birth year after 1900 (u8).
    /// @param birthMonth Birth month (u8).
    /// @param birthDay Birth day (u8).
    /// @return The encoded payload.
    core::Bytes profileRequest(
        int heartNumber,
        int bloodType,
        int birthYear,
        int birthMonth,
        int birthDay
    );

    /// @brief Builds 10/3: the profile was handed in (the server's gift).
    /// @return The encoded payload.
    core::Bytes profileDoneRequest();

}

#endif
