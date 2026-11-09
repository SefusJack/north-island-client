#ifndef NORTH_ISLAND_GFX_MOUSE_CURSOR_H
#define NORTH_ISLAND_GFX_MOUSE_CURSOR_H

#include "formats/AnimatedCursor.h"
#include "formats/PixelArtScale.h"
#include "gfx/SdlDeleter.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace gfx {

    /// @brief A system mouse cursor made from an animated cursor file; update() switches frames when their time is up.
    class MouseCursor {
        public:
            /// @brief Builds the cursor frames from an animated cursor.
            /// @param source The animated cursor.
            /// @param scale The frames that many times as big each way (by the pixel-art filter), the hotspot with them.
            /// @param filter The pixel-art filter used to scale.
            /// @return The cursor, or nothing when a frame cannot be made.
            static std::optional<MouseCursor> create(
                const formats::AnimatedCursor& source,
                int scale = 1,
                formats::PixelArtFilter filter = formats::PixelArtFilter::None
            );

            /// @brief Makes this the system cursor and restarts its animation.
            /// @param nowMs The current time in milliseconds.
            void activate(std::uint64_t nowMs);

            /// @brief Switches to the next frame when the current one's time is up.
            /// @param nowMs The current time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            std::vector<SdlHandle<SDL_Cursor>> m_frames;
            std::vector<formats::AnimatedCursor::Step> m_steps;
            std::size_t m_step = 0;
            std::uint64_t m_stepStartMs = 0;
    };

}

#endif
