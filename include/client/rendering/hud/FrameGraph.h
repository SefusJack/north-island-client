#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_FRAME_GRAPH_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_FRAME_GRAPH_H

#include "client/services/frame/FrameStats.h"
#include "gfx/Canvas.h"

#include <string>

namespace client {

    /// @brief The frame time overlay (F11 or --perf): stacked CPU bars of the last frames with GPU dots and the means in numbers.
    /// @details Not part of the original game; drawn over everything at the top right of the screen.
    class FrameGraph {
        public:
            static constexpr int kWidth = 320; // width of the overlay box, px

            /// @brief Draws the overlay.
            /// @param canvas Target canvas.
            /// @param stats The recorded frame samples.
            /// @param backend The renderer's name.
            /// @param gpuMeasured Whether that renderer has a GPU timer.
            /// @param screenWidth Width of the screen in pixels, the overlay sits at its right edge.
            static void draw(
                gfx::Canvas& canvas,
                const FrameStats& stats,
                const std::string& backend,
                bool gpuMeasured,
                int screenWidth
            );
    };

}

#endif
