#ifndef NORTH_ISLAND_PROTOCOL_INSTANCE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_INSTANCE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds 85/1: ask for the list of open instances.
    /// @details Original: client action 85, the instances ("duplicate missions", SendServerMsg 85/x in DZ_Connect.pas).
    /// @return The encoded payload.
    core::Bytes instanceListRequest();

    /// @brief Builds 85/2: ask for a page of the list (5 rows a page).
    /// @param page Page number.
    /// @return The encoded payload.
    core::Bytes instancePageRequest(int page);

    /// @brief Builds 85/3: create the instance of a scene with a message of up to 28 characters.
    /// @param sceneId Scene to create the instance of.
    /// @param message Message text.
    /// @return The encoded payload.
    core::Bytes instanceCreateRequest(int sceneId, const std::string& message);

    /// @brief Builds 85/4: ask for the detail window of an open instance (its runtime map).
    /// @param mapId Runtime map of the instance.
    /// @return The encoded payload.
    core::Bytes instanceDetailRequest(int mapId);

    /// @brief Builds 85/5: join the instance shown in the detail window.
    /// @param mapId Runtime map of the instance.
    /// @return The encoded payload.
    core::Bytes instanceJoinRequest(int mapId);

    /// @brief Builds 85/6: leave the own instance.
    /// @return The encoded payload.
    core::Bytes instanceLeaveRequest();

    /// @brief Builds 85/9: go into the own instance.
    /// @return The encoded payload.
    core::Bytes instanceEnterRequest();

    /// @brief Builds 85/10: the members tab was opened.
    /// @return The encoded payload.
    core::Bytes instanceMembersRequest();

    /// @brief Builds 85/11: ask for a page of the members (5 a page).
    /// @param page Page number.
    /// @return The encoded payload.
    core::Bytes instanceMembersPageRequest(int page);

    /// @brief Builds 85/13: the creator sends a member away while the instance waits.
    /// @param mapId Runtime map of the instance.
    /// @param memberId Player id of the member.
    /// @return The encoded payload.
    core::Bytes instanceKickRequest(int mapId, std::int32_t memberId);

}

#endif
