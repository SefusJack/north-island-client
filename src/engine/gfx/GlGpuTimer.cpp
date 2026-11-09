#include "gfx/GlGpuTimer.h"

namespace gfx {

    namespace {

        constexpr unsigned int kTimeElapsed = 0x88BF; // OpenGL enum GL_TIME_ELAPSED (query target)
        constexpr unsigned int kQueryResult = 0x8866; // OpenGL enum GL_QUERY_RESULT
        constexpr unsigned int kQueryResultAvailable = 0x8867; // OpenGL enum GL_QUERY_RESULT_AVAILABLE

        /// @brief Looks an OpenGL function up by name through SDL.
        /// @tparam Function The function pointer type.
        /// @param function Receives the function pointer.
        /// @param name The OpenGL function name.
        /// @return True when the function was found.
        template <typename Function>
        bool load(Function& function, const char* name) {
            function = reinterpret_cast<Function>(SDL_GL_GetProcAddress(name));
            return function != nullptr;
        }

    }

    std::unique_ptr<GpuTimer> GlGpuTimer::create(SDL_Renderer* renderer) {
        if (!SDL_GL_ExtensionSupported("GL_ARB_timer_query")) {
            return nullptr;
        }
        std::unique_ptr<GlGpuTimer> timer(new GlGpuTimer(renderer));
        if (!load(timer->m_genQueries, "glGenQueries") || !load(timer->m_deleteQueries, "glDeleteQueries") || !load(timer->m_beginQuery, "glBeginQuery") ||
            !load(timer->m_endQuery, "glEndQuery") || !load(timer->m_getQueryObjectiv, "glGetQueryObjectiv") ||
            !load(timer->m_getQueryObjectui64v, "glGetQueryObjectui64v")) {
            return nullptr;
        }
        timer->m_genQueries(kSlots, timer->m_queries.data());
        return timer;
    }

    GlGpuTimer::~GlGpuTimer() {
        if (m_deleteQueries != nullptr && m_queries[0] != 0) {
            m_deleteQueries(kSlots, m_queries.data());
        }
    }

    void GlGpuTimer::collect() {
        for (int i = 0; i < kSlots; ++i) {
            const int at = (m_next + i) % kSlots;
            if (!m_pending[at]) {
                continue;
            }
            int available = 0;
            m_getQueryObjectiv(m_queries[at], kQueryResultAvailable, &available);
            if (available == 0) {
                continue;
            }
            std::uint64_t elapsed = 0;
            m_getQueryObjectui64v(m_queries[at], kQueryResult, &elapsed);
            m_pending[at] = false;
            m_latestMs = static_cast<double>(elapsed) / 1.0e6; // nanoseconds to milliseconds
        }
    }

    void GlGpuTimer::beginFrame() {
        collect();
        m_active = !m_pending[m_next];
        if (!m_active) {
            return;
        }
        SDL_FlushRenderer(m_renderer);
        m_beginQuery(kTimeElapsed, m_queries[m_next]);
    }

    void GlGpuTimer::endFrame() {
        if (!m_active) {
            return;
        }
        SDL_FlushRenderer(m_renderer);
        m_endQuery(kTimeElapsed);
        m_pending[m_next] = true;
        m_next = (m_next + 1) % kSlots;
        m_active = false;
    }

}
