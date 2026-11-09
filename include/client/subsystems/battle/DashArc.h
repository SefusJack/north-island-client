#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_DASH_ARC_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_DASH_ARC_H

#include "core/Point.h"

namespace client {

    /// @brief The height of a fighter running onto its cell above the ground.
    /// @details Original: ApplyJumpArc, run by DrawAfterImages before the sprite while FDashing is set by DashFightHumTo.
    class DashArc {
        public:
            /// @brief Returns the length of the dash way.
            /// @details Original: FDashDist, the Round of the distance from the start to the cell.
            /// @param start Start point.
            /// @param cell Target cell.
            /// @return The length in pixels.
            static int length(core::Point start, core::Point cell);

            /// @brief Returns the lift of the sprite for the distance still to go.
            /// @details Original: FJumpArcHeight, Round((0.5 - Abs(Dist / (FDashDist + 0.001) - 0.5)) * 360), less 0.003 of its square.
            /// @param position Current position.
            /// @param cell Target cell.
            /// @param length Length of the whole way (see length).
            /// @return The lift in pixels.
            static int lift(core::Point position, core::Point cell, int length);
    };

}

#endif
