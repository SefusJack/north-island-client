#ifndef NORTH_ISLAND_CLIENT_RULES_WALKING_H
#define NORTH_ISLAND_CLIENT_RULES_WALKING_H

#include "client/models/world/MapCharacter.h"
#include "core/Point.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace client {

    /// @brief Gives the direction from one point to another (Original: CalcDirection).
    /// @details 0 north, 1 north-west, ... 7 north-east; diagonal when the slope lies between 0.25 and 2.
    /// @param from Start point.
    /// @param to End point.
    /// @param current Direction returned when the points are the same.
    /// @return The direction 0..7.
    int directionTowards(core::Point from, core::Point to, int current);

    /// @brief Sets a character off along a path.
    /// @param character The character to move.
    /// @param path Corners from findPath.
    /// @param nowMs Current time in ms.
    /// @param arrivalDirection Which way to face at the end (the server's walks say so; the own walks keep the last direction).
    void startWalking(MapCharacter& character, std::vector<core::Point> path, std::uint64_t nowMs, std::optional<int> arrivalDirection = std::nullopt);

    /// @brief How a walking step is measured: Whole (the original's rounding) or SubPixel (any frame length).
    enum class WalkSteps { Whole, SubPixel };

    /// @brief Moves a walking character on by the time since its last step (Original: THuman.UpdateMove).
    /// @param character The walking character.
    /// @param nowMs Current time in ms.
    /// @param steps How the step is measured.
    /// @return The step the camera follows (how far it moved, but none on arriving at a corner).
    core::Point walkStep(MapCharacter& character, std::uint64_t nowMs, WalkSteps steps = WalkSteps::Whole);

}

#endif
