#ifndef NORTH_ISLAND_GFX_GL_GPU_TIMER_H
#define NORTH_ISLAND_GFX_GL_GPU_TIMER_H

#include "gfx/GpuTimer.h"

#include <array>
#include <cstdint>

// The calling convention of OpenGL functions: __stdcall on Windows (APIENTRY), which differs from SDL's __cdecl in 32-bit
// builds. Called as __cdecl there, each call left the stack unbalanced and the OpenGL renderer never got past startup.
#ifdef _WIN32
#define NORTH_ISLAND_GL_CALL __stdcall
#else
#define NORTH_ISLAND_GL_CALL
#endif

namespace gfx {

    /// @brief GPU timer of the "opengl" renderer: a GL_TIME_ELAPSED query a frame (OpenGL 3.3 or ARB_timer_query).
    class GlGpuTimer final : public GpuTimer {
        public:
            /// @brief Creates the timer for an OpenGL renderer.
            /// @param renderer The SDL renderer.
            /// @return The timer, or nothing when the query functions are missing.
            static std::unique_ptr<GpuTimer> create(SDL_Renderer* renderer);

            /// @brief Deletes the queries.
            ~GlGpuTimer() override;

            /// @brief Starts the time query before a frame's drawing.
            void beginFrame() override;

            /// @brief Ends the time query before the present.
            void endFrame() override;

        private:
            using GenQueries = void(NORTH_ISLAND_GL_CALL*)(int count, unsigned int* ids);
            using DeleteQueries = void(NORTH_ISLAND_GL_CALL*)(int count, const unsigned int* ids);
            using BeginQuery = void(NORTH_ISLAND_GL_CALL*)(unsigned int target, unsigned int id);
            using EndQuery = void(NORTH_ISLAND_GL_CALL*)(unsigned int target);
            using GetQueryObjectiv = void(NORTH_ISLAND_GL_CALL*)(unsigned int id, unsigned int name, int* value);
            using GetQueryObjectui64v = void(NORTH_ISLAND_GL_CALL*)(unsigned int id, unsigned int name, std::uint64_t* value);

            /// @brief Wraps the renderer.
            /// @param renderer The SDL renderer.
            explicit GlGpuTimer(
                SDL_Renderer* renderer
            )
                : m_renderer(renderer) {}

            /// @brief Reads the queries of finished frames without waiting.
            void collect();

            SDL_Renderer* m_renderer;
            GenQueries m_genQueries = nullptr;
            DeleteQueries m_deleteQueries = nullptr;
            BeginQuery m_beginQuery = nullptr;
            EndQuery m_endQuery = nullptr;
            GetQueryObjectiv m_getQueryObjectiv = nullptr;
            GetQueryObjectui64v m_getQueryObjectui64v = nullptr;
            std::array<unsigned int, kSlots> m_queries{};
            std::array<bool, kSlots> m_pending{};
            int m_next = 0;
            bool m_active = false;
    };

}

#endif
