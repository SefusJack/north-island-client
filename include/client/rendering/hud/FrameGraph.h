#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_FRAME_GRAPH_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_FRAME_GRAPH_H

#include "client/services/frame/FrameStats.h"
#include "client/services/frame/MachineLoad.h"
#include "gfx/Canvas.h"

#include <cstdint>
#include <string>
#include <vector>

namespace client {

    /// @brief The performance overlay (F11, --perf or the Video tab), at the right edge of the screen below the function buttons, over everything.
    /// @details Level 1 shows the frames a second; level 2 adds how much of the processor, the
    /// graphics card and the RAM the client uses and how long processor and graphics card work on a frame. Level 3 adds the
    /// frame's parts in numbers and the frame time graph (stacked CPU bars of the last frames with GPU dots).
    class FrameGraph {
        public:
            static constexpr int kWidth = 340; // width of the level 3 box, px (its longest line is about 40 characters)
            static constexpr int kLevels = 4; // off and levels 1 to 3

            /// @brief Draws the overlay.
            /// @param canvas Target canvas.
            /// @param stats The recorded frame samples.
            /// @param backend The renderer's name.
            /// @param gpuMeasured Whether that renderer has a GPU timer.
            /// @param screenWidth Width of the screen in pixels.
            /// @param hudPercent The HUD size in percent (100 to 200): the function buttons at the top right, which the overlay
            /// sits below, grow with it.
            /// @param level 1, 2 or 3 (see the class).
            /// @param load The client's processor and graphics card use (levels 2 and 3).
            /// @param nowMs The frame's time; the numbers are worded anew four times a second so they can be read.
            void draw(
                gfx::Canvas& canvas,
                const FrameStats& stats,
                const std::string& backend,
                bool gpuMeasured,
                int screenWidth,
                int hudPercent,
                int level,
                const LoadReading& load,
                std::uint64_t nowMs
            );

        private:
            std::vector<std::string> m_lines; // the numbers as last worded
            int m_linesLevel = 0; // the level they were worded for
            std::uint64_t m_wordedMs = 0;
    };

}

#endif
