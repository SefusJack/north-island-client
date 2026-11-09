#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_WALK_MARK_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_WALK_MARK_H

#include "core/Point.h"

#include <cstdint>
#include <optional>

namespace client {

    /// @brief The arrow that flashes at the destination of a walk click: pictures Arrow2, Arrow3 and Arrow4 (64 x 64) in turn, 120 ms each.
    /// @details Original: DZ_Game.pas WalkMarkActive / MouseArrowPlay. Like the game, a start only resets the frame to 1: the picture and
    /// the tick of the last frame stay, so the first frame shown is the one after the old tick has run out, and a click during a run
    /// keeps the old picture at the new place until its time is up.
    class WalkMark {
        public:
            /// @brief Starts the arrow at a walk click's destination.
            /// @details Original: WalkMarkActive, the destination and frame 1.
            /// @param destination Map position of the click.
            void start(core::Point destination);

            /// @brief Advances the arrow animation.
            /// @details Original: MouseArrowPlay.
            /// @param nowMs Current time in ms.
            /// @return The number of the Arrow picture to draw at position() now, 0 for none.
            int update(std::uint64_t nowMs);

            /// @brief Where the arrow is drawn.
            /// @return The map position.
            core::Point position() const { return m_position; }

        private:
            bool m_active = false;
            core::Point m_position;
            int m_frame = 1;
            int m_shown = 0;
            std::optional<std::uint64_t> m_tickMs;
    };

}

#endif
