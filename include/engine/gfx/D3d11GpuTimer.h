#ifndef NORTH_ISLAND_GFX_D3D11_GPU_TIMER_H
#define NORTH_ISLAND_GFX_D3D11_GPU_TIMER_H

#include "gfx/GpuTimer.h"

#include <array>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Query;

namespace gfx {

    /// @brief GPU timer of the "direct3d11" renderer: a disjoint query and two timestamp queries a frame (Windows only).
    class D3d11GpuTimer final : public GpuTimer {
        public:
            /// @brief Creates the timer for a Direct3D 11 renderer.
            /// @param renderer The SDL renderer.
            /// @return The timer, or nothing when the queries cannot be created.
            static std::unique_ptr<GpuTimer> create(SDL_Renderer* renderer);

            /// @brief Releases the queries.
            ~D3d11GpuTimer() override;

            /// @brief Starts the timestamp queries before a frame's drawing.
            void beginFrame() override;

            /// @brief Ends the queries before the present.
            void endFrame() override;

        private:
            /// @brief The queries of one frame in flight.
            struct Slot {
                ID3D11Query* disjoint = nullptr;
                ID3D11Query* start = nullptr;
                ID3D11Query* end = nullptr;
                bool pending = false;
            };

            /// @brief Wraps the renderer's device.
            /// @param renderer The SDL renderer.
            /// @param device The Direct3D 11 device.
            /// @param context The device context.
            D3d11GpuTimer(
                SDL_Renderer* renderer,
                ID3D11Device* device,
                ID3D11DeviceContext* context
            )
                : m_renderer(renderer),
                  m_device(device),
                  m_context(context) {}

            /// @brief Creates the query slots.
            /// @return True when all queries were created.
            bool createQueries();

            /// @brief Reads the queries of finished frames without waiting.
            void collect();

            SDL_Renderer* m_renderer;
            ID3D11Device* m_device;
            ID3D11DeviceContext* m_context;
            std::array<Slot, kSlots> m_slots{};
            int m_next = 0;
            bool m_active = false;
    };

}

#endif
