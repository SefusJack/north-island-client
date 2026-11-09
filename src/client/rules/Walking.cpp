#include "client/rules/Walking.h"

#include "client/rules/CharacterActions.h"

#include <cmath>
#include <cstdlib>

namespace client {

    int directionTowards(core::Point from, core::Point to, int current) {
        // 0.001 avoids a division by zero; slopes 0.25..2 count as diagonal
        const double slope = std::abs(to.y - from.y) / (std::abs(to.x - from.x) + 0.001);
        const bool diagonal = slope > 0.25 && slope < 2;
        if (diagonal && to.x < from.x && to.y > from.y) {
            return 3; // direction 3: south-west
        }
        if (diagonal && to.x < from.x && to.y < from.y) {
            return 1; // direction 1: north-west
        }
        if (diagonal && to.x > from.x && to.y > from.y) {
            return 5; // direction 5: south-east
        }
        if (diagonal && to.x > from.x && to.y < from.y) {
            return 7; // direction 7: north-east
        }
        if (to.x > from.x && slope <= 0.25) { // 0.25: diagonal slope limit
            return 6; // direction 6: east
        }
        if (to.x < from.x && slope <= 0.25) { // 0.25: diagonal slope limit
            return 2; // direction 2: west
        }
        if (to.y > from.y) {
            return 4; // direction 4: south
        }
        if (to.y < from.y) {
            return 0; // direction 0: north
        }
        return current;
    }

    void startWalking(MapCharacter& character, std::vector<core::Point> path, std::uint64_t nowMs, std::optional<int> arrivalDirection) {
        character.path = std::move(path);
        character.pathIndex = 0;
        character.stepMs = nowMs;
        character.arrivalDirection = arrivalDirection;
        character.walking = !character.path.empty();
        if (character.walking) {
            stopAction(character);
            character.direction = directionTowards(character.position, character.path.front(), character.direction);
        }
    }

    namespace {

        constexpr double kBeatMs = 30; // the original's walk timer beat, ms: one Whole step per beat

        /// @brief Handles reaching a corner: turns to the next one, or stops (facing the arrival direction) at the last.
        /// @param character The walking character.
        /// @param corner The corner just reached.
        void arrive(MapCharacter& character, core::Point corner) {
            ++character.pathIndex;
            if (character.pathIndex < character.path.size()) {
                character.direction = directionTowards(corner, character.path[character.pathIndex], character.direction);
            } else {
                character.walking = false;
                if (character.arrivalDirection) {
                    character.direction = *character.arrivalDirection;
                }
            }
        }

        /// @brief Moves a character for WalkSteps::SubPixel, as far as the elapsed time goes at the pace a Whole step has at the 30 ms beat, through as many corners as that reaches.
        /// @param character The walking character.
        /// @param nowMs Current time in ms.
        /// @return The step the camera follows (none on arriving at a corner).
        core::Point fineStep(MapCharacter& character, std::uint64_t nowMs) {
            const double elapsed = static_cast<double>(nowMs - character.stepMs);
            character.stepMs = nowMs;
            const double pace = std::lround(character.pixelsPerMs * kBeatMs) / kBeatMs;
            FinePosition fine = character.fine && character.fine->rounded == character.position
                                    ? *character.fine
                                    : FinePosition{static_cast<double>(character.position.x), static_cast<double>(character.position.y), character.position};
            const core::Point before = character.position;
            double left = pace * elapsed;
            bool arrived = false;
            while (left > 0 && character.walking && character.pathIndex < character.path.size()) {
                const core::Point corner = character.path[character.pathIndex];
                const double dx = corner.x - fine.x;
                const double dy = corner.y - fine.y;
                const double distance = std::sqrt(dx * dx + dy * dy);
                if (distance <= left) {
                    fine.x = corner.x;
                    fine.y = corner.y;
                    left -= distance;
                    arrived = true;
                    arrive(character, corner);
                } else {
                    fine.x += dx / distance * left;
                    fine.y += dy / distance * left;
                    left = 0;
                }
            }
            character.position = {static_cast<int>(std::lround(fine.x)), static_cast<int>(std::lround(fine.y))};
            fine.rounded = character.position;
            character.fine = character.walking ? std::optional<FinePosition>(fine) : std::nullopt;
            return arrived ? core::Point{} : core::Point{character.position.x - before.x, character.position.y - before.y};
        }

    }

    core::Point walkStep(MapCharacter& character, std::uint64_t nowMs, WalkSteps steps) {
        if (!character.walking || character.pathIndex >= character.path.size()) {
            character.walking = false;
            character.fine.reset();
            return {};
        }
        if (steps == WalkSteps::SubPixel) {
            return fineStep(character, nowMs);
        }
        const int speed = static_cast<int>(std::lround(character.pixelsPerMs * static_cast<double>(nowMs - character.stepMs)));
        character.stepMs = nowMs;
        const core::Point target = character.path[character.pathIndex];
        const core::Point before = character.position;
        const int dx = target.x - before.x;
        const int dy = target.y - before.y;
        bool arrived = false;
        if (std::abs(dx) <= speed && std::abs(dy) <= speed) {
            character.position = target;
            arrived = true;
            arrive(character, target);
        } else {
            // 0.001 avoids a division by zero
            const double ratio = std::abs(dy / (0.001 + dx));
            const double along = 1 / std::sqrt(ratio * ratio + 1) * speed;
            const int stepX = static_cast<int>(std::lround(along));
            const int stepY = static_cast<int>(std::lround(ratio * along));
            character.position.x += dx >= 0 ? stepX : -stepX;
            character.position.y += dy >= 0 ? stepY : -stepY;
        }
        return arrived ? core::Point{} : core::Point{character.position.x - before.x, character.position.y - before.y};
    }

}
