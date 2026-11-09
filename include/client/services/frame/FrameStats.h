#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_STATS_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_STATS_H

#include <array>
#include <cstddef>
#include <optional>

namespace client {

    /// @brief One main-loop pass's timings in milliseconds: CPU phases in order, plus the GPU time when measurable.
    struct FrameSample {
        double eventsMs = 0;
        double updateMs = 0;
        double drawMs = 0;
        double presentMs = 0;
        double waitMs = 0;
        double totalMs = 0;
        std::optional<double> gpuMs;

        /// @brief The CPU's own work, everything but the waiting.
        /// @return Sum of events, update, draw and present time in milliseconds.
        double busyMs() const { return eventsMs + updateMs + drawMs + presentMs; }
    };

    /// @brief Ring buffer of the last frames of the main loop (the frame time graph).
    class FrameStats {
        public:
            static constexpr std::size_t kCapacity = 240; // frames kept (one graph column each)

            /// @brief Stores a frame, dropping the oldest when full.
            /// @param sample The frame's timings.
            void add(const FrameSample& sample);

            /// @brief Number of kept frames.
            /// @return The count.
            std::size_t size() const { return m_size; }

            /// @brief Gets a kept frame by age order.
            /// @param index 0 is the oldest kept.
            /// @return The sample.
            const FrameSample& at(std::size_t index) const { return m_samples[(m_next + kCapacity - m_size + index) % kCapacity]; }

            /// @brief The newest kept frame.
            /// @return The sample.
            const FrameSample& latest() const { return at(m_size - 1); }

            /// @brief Mean over the kept frames.
            /// @return The averaged sample (gpu averaged over frames that measured it).
            FrameSample average() const;

            /// @brief Frames a second from the mean frame time.
            /// @return The frame rate, 0 when nothing is kept.
            double fps() const;

            /// @brief Frames a second from the mean of the slowest 1% of frames (at least one frame).
            /// @return The frame rate, 0 when nothing is kept.
            double lowFps() const;

        private:
            std::array<FrameSample, kCapacity> m_samples{};
            std::size_t m_next = 0;
            std::size_t m_size = 0;
    };

}

#endif
