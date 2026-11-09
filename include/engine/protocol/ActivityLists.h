#ifndef NORTH_ISLAND_PROTOCOL_ACTIVITY_LISTS_H
#define NORTH_ISLAND_PROTOCOL_ACTIVITY_LISTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace protocol {

    /// @brief One event of the schedule.
    /// @details Original: TActionInfo.
    struct ActivityRecord {
        int weekDay = 0;
        std::string name;
        double time = 0.0;
        int duration = 0;
        int mapId = 0;
        int mapX = 0;
        int mapY = 0;
        int minLevel = 0;
        int state = 1;
        std::string description;
    };

    /// @brief Server 90/0: the whole event schedule.
    /// @details Original: TActionListForm.RecvActionPacket.
    struct ActivityList {
        int weekDay = 0;
        std::int32_t version = 0;
        std::vector<ActivityRecord> activities;
    };

    /// @brief Decodes server 90/0: u8 weekday, i32 version, then the events.
    /// @param payload Packet payload.
    /// @return The schedule, or nothing when malformed.
    std::optional<ActivityList> decodeActivityList(const core::Bytes& payload);

    /// @brief Server 90/1: event states as pairs of number (from 1) and state; a leading 0 and a weekday start the day anew.
    struct ActivityStates {
        bool newDay = false;
        int weekDay = 0;
        std::vector<std::pair<int, int>> states;
    };

    /// @brief Decodes server 90/1: the event state changes.
    /// @param payload Packet payload.
    /// @return The states, or nothing when malformed.
    std::optional<ActivityStates> decodeActivityStates(const core::Bytes& payload);

}

#endif
