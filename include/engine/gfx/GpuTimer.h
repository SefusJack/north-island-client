#ifndef NORTH_ISLAND_GFX_GPU_TIMER_H
#define NORTH_ISLAND_GFX_GPU_TIMER_H

#include <SDL3/SDL.h>

#include <memory>
#include <optional>
#include <string>

namespace gfx {

    /// @brief Measures how long the GPU took for what a frame drew, from the renderer's own timer queries.
    /// @details A query is read a few frames after it was issued and never waited for.
    class GpuTimer {
        public:
            /// @brief Creates the timer that fits a renderer backend.
            /// @param renderer The SDL renderer.
            /// @param backendName The renderer's name ("direct3d", "direct3d11" or "opengl").
            /// @return The timer, or nothing when the renderer cannot be measured.
            static std::unique_ptr<GpuTimer> create(SDL_Renderer* renderer, const std::string& backendName);

            virtual ~GpuTimer() = default;
            GpuTimer(const GpuTimer&) = delete;
            GpuTimer& operator=(const GpuTimer&) = delete;

            /// @brief Called before the first drawing of a frame.
            virtual void beginFrame() = 0;

            /// @brief Called before the present.
            virtual void endFrame() = 0;

            /// @brief The GPU time of the newest frame the GPU has finished.
            /// @return Milliseconds, or nothing when none is known yet.
            std::optional<double> latestMs() const { return m_latestMs; }

        protected:
            GpuTimer() = default;

            static constexpr int kSlots = 4;
            std::optional<double> m_latestMs;
    };

}

#endif
