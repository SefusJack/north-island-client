#include "client/rules/NpcMovement.h"

#include "client/rules/Walking.h"

#include <algorithm>

namespace client {

    namespace {

        /// @brief Advances a patrol to its next point.
        /// @param routine The NPC's routine.
        /// @return True when there is a next point, false when the patrol stops after the last one.
        bool nextPatrolPoint(NpcRoutine& routine) {
            if (routine.index + 1 < routine.points.size()) {
                ++routine.index;
                return true;
            }
            switch (routine.end) {
                case NpcRoutine::End::Again: // patrol ends by starting over at the first point
                    routine.index = 0;
                    return true;
                case NpcRoutine::End::Back: // patrol ends by walking back: the points are turned after the first round
                    std::reverse(routine.points.begin() + 1, routine.points.end());
                    std::reverse(routine.waits.begin() + 1, routine.waits.end());
                    routine.index = 0;
                    return true;
                case NpcRoutine::End::Stop: // patrol ends and the NPC stands
                    break;
            }
            return false;
        }

        /// @brief Picks a random point inside an area.
        /// @param area The wander area.
        /// @param random Random source.
        /// @return A point in the area.
        core::Point randomPoint(const core::Rect& area, std::mt19937& random) {
            const auto pick = [&](int from, int size) { return size > 0 ? from + std::uniform_int_distribution<int>(0, size - 1)(random) : from; };
            return {pick(area.x, area.width), pick(area.y, area.height)};
        }

    }

    void moveNpc(MapNpc& npc, std::uint64_t nowMs, std::mt19937& random, WalkSteps steps) {
        MapCharacter& body = npc.body;
        if (npc.held) {
            body.walking = false;
            npc.routine.arrivedMs = 0;
            return;
        }
        if (body.walking) {
            walkStep(body, nowMs, steps);
            return;
        }
        NpcRoutine& routine = npc.routine;
        if (routine.kind == NpcRoutine::Kind::Stand) {
            return;
        }
        if (routine.arrivedMs == 0) {
            routine.arrivedMs = nowMs;
        }
        const int wait = routine.kind == NpcRoutine::Kind::Patrol && routine.index < routine.waits.size() ? routine.waits[routine.index] : routine.wanderWaitMs;
        if (nowMs - routine.arrivedMs < static_cast<std::uint64_t>(std::max(0, wait))) {
            return;
        }
        core::Point next;
        if (routine.kind == NpcRoutine::Kind::Patrol) {
            if (!nextPatrolPoint(routine)) {
                return;
            }
            next = routine.points[routine.index];
        } else {
            next = randomPoint(routine.area, random);
        }
        routine.arrivedMs = 0;
        startWalking(body, {next}, nowMs);
    }

}
