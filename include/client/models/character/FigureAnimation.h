#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_FIGURE_ANIMATION_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_FIGURE_ANIMATION_H

#include <cstdint>

namespace client {

    /// @brief The frame of a figure: the next one every 230 ms standing or 100 ms walking, round the action's frames.
    class FigureAnimation {
        public:
            /// @brief The frame to show now, advancing at 230 ms standing or 100 ms walking.
            /// @param nowMs Current time in ms.
            /// @param frameCount Number of frames of the action.
            /// @param walking Whether the figure walks.
            /// @return Frame index.
            int frame(std::uint64_t nowMs, int frameCount, bool walking = false);

            /// @brief The frame to show now at another pace, optionally stopping on the last frame.
            /// @param nowMs Current time in ms.
            /// @param frameCount Number of frames of the action.
            /// @param frameMs Milliseconds per frame.
            /// @param holdLast Whether to stop on the last frame.
            /// @return Frame index.
            int frame(std::uint64_t nowMs, int frameCount, std::uint64_t frameMs, bool holdLast);

            /// @brief Restarts from frame 0.
            void restart() { m_frame = 0; }

            /// @brief Starts from a given frame.
            /// @param frame Frame index to show.
            void startAt(int frame) { m_frame = frame; }

            /// @brief The frame shown now (FAnimFrame).
            /// @return Frame index.
            int current() const { return m_frame; }

        private:
            int m_frame = 0;
            std::uint64_t m_frameMs = 0;
    };

}

#endif
