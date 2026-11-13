#ifndef NORTH_ISLAND_CORE_WORKER_PRIORITY_H
#define NORTH_ISLAND_CORE_WORKER_PRIORITY_H

// Speedups::quietWorkers: worker threads that decode pictures and music ahead run below the main thread's priority.
// On a single core Windows gives each normal-priority worker a whole turn (tens of ms on Windows XP), one after the other,
// and the frame being drawn stands still for that long (a 46 x 72 texture "took" 101 ms to make at battle start). Below
// normal, a worker only runs while the main thread waits: in a frame-cap wait, in the few ms the main thread gives it each
// frame (workerSliceMs), or when the main thread needs its result and blocks on it.

#include "core/Speedups.h"

#ifdef _WIN32
extern "C" {
__declspec(dllimport) void* __stdcall GetCurrentThread();
__declspec(dllimport) int __stdcall GetThreadPriority(void* thread);
__declspec(dllimport) int __stdcall SetThreadPriority(void* thread, int priority);
}
#endif

namespace core {

    /// @brief How long the main thread waits for a pending worker each frame.
    /// @return 4 ms with quiet workers, else 0 (only a look).
    inline int workerSliceMs() { return speedups().quietWorkers ? 4 : 0; }

    /// @brief Runs the current (pool) thread below normal priority while this lives, then puts its priority back.
    class LowPriorityScope {
        public:
            LowPriorityScope() {
#ifdef _WIN32
                if (speedups().quietWorkers) {
                    m_before = GetThreadPriority(GetCurrentThread());
                    m_changed = SetThreadPriority(GetCurrentThread(), -1) != 0; // -1: THREAD_PRIORITY_BELOW_NORMAL
                }
#endif
            }
            ~LowPriorityScope() {
#ifdef _WIN32
                if (m_changed) {
                    SetThreadPriority(GetCurrentThread(), m_before);
                }
#endif
            }
            LowPriorityScope(const LowPriorityScope&) = delete;
            LowPriorityScope& operator=(const LowPriorityScope&) = delete;

        private:
            int m_before = 0;
            bool m_changed = false;
    };

}

#endif
