#ifndef NORTH_ISLAND_GFX_D3D9_GPU_TIMER_H
#define NORTH_ISLAND_GFX_D3D9_GPU_TIMER_H

#include "gfx/GpuTimer.h"

#include <array>

struct IDirect3DDevice9;
struct IDirect3DQuery9;

namespace gfx {

    /// @brief GPU timer of the "direct3d" renderer: a disjoint, a frequency and two timestamp queries a frame (Windows only).
    class D3d9GpuTimer final : public GpuTimer {
        public:
            /// @brief Creates the timer for a Direct3D 9 renderer.
            /// @param renderer The SDL renderer.
            /// @return The timer, or nothing when the queries cannot be created.
            static std::unique_ptr<GpuTimer> create(SDL_Renderer* renderer);

            /// @brief Releases the queries.
            ~D3d9GpuTimer() override;

            /// @brief Starts the timestamp queries before a frame's drawing.
            void beginFrame() override;

            /// @brief Ends the queries before the present.
            void endFrame() override;

        private:
            /// @brief The queries of one frame in flight.
            struct Slot {
                IDirect3DQuery9* disjoint = nullptr;
                IDirect3DQuery9* frequency = nullptr;
                IDirect3DQuery9* start = nullptr;
                IDirect3DQuery9* end = nullptr;
                bool pending = false;
            };

            /// @brief Wraps the renderer's device.
            /// @param renderer The SDL renderer.
            /// @param device The Direct3D 9 device.
            D3d9GpuTimer(
                SDL_Renderer* renderer,
                IDirect3DDevice9* device
            )
                : m_renderer(renderer),
                  m_device(device) {}

            /// @brief Creates the query slots.
            /// @return True when all queries were created.
            bool createQueries();

            /// @brief Reads the queries of finished frames without waiting.
            void collect();

            SDL_Renderer* m_renderer;
            IDirect3DDevice9* m_device;
            std::array<Slot, kSlots> m_slots{};
            int m_next = 0;
            bool m_active = false;
    };

}

#endif
