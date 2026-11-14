#ifndef NORTH_ISLAND_CORE_BACKGROUND_LOAD_H
#define NORTH_ISLAND_CORE_BACKGROUND_LOAD_H

#include <chrono>
#include <functional>
#include <future>
#include <utility>

namespace core {

    /// @brief One piece of work run on another thread whose result the frame loop picks up once done; destroying it waits for the work.
    /// @tparam T Type of the work's result.
    template <class T>
    class BackgroundLoad {
        public:
            /// @brief Runs work on a new thread; a load still pending is waited for first.
            /// @param work The work to run.
            void start(std::function<T()> work) { m_result = std::async(std::launch::async, std::move(work)); }

            /// @brief Tells whether work was started and its result not taken yet.
            /// @return True when a result is pending.
            bool pending() const { return m_result.valid(); }

            /// @brief Tells whether the work has ended.
            /// @return True when the result can be taken, false when nothing is pending.
            bool ready() const { return m_result.valid() && m_result.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

            /// @brief Tells whether the work has ended, waiting up to some ms for it (the waiting lets a low-priority worker run).
            /// @param ms The longest wait.
            bool readyWithin(int ms) const { return m_result.valid() && m_result.wait_for(std::chrono::milliseconds(ms)) == std::future_status::ready; }

            /// @brief Takes the result of the work, waiting when it is not ready.
            /// @return The work's result.
            T take() { return m_result.get(); }

        private:
            std::future<T> m_result;
    };

}

#endif
