#ifndef NORTH_ISLAND_PROTOCOL_STAT_POINTS_REQUEST_H
#define NORTH_ISLAND_PROTOCOL_STAT_POINTS_REQUEST_H

#include "core/Bytes.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace protocol {

    /// @brief Builds request 8/1: spend stat points.
    /// @details Original: TSe_EquipForm2.SendStatPoints. Layout: 08 01 00, u8 count, count x (u8 attribute, i32 points); attributes STR 28, CON 29, INT 27, WIS 33, AGI 30.
    /// @param points Pairs of attribute id and points to add.
    /// @return The encoded request payload.
    core::Bytes statPointsRequest(const std::vector<std::pair<int, std::int32_t>>& points);

}

#endif
