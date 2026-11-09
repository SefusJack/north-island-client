#ifndef NORTH_ISLAND_FORMATS_ANIMATED_CURSOR_H
#define NORTH_ISLAND_FORMATS_ANIMATED_CURSOR_H

#include "core/Bytes.h"
#include "formats/CursorFrame.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace formats {

    /// @brief A Windows animated cursor (.ani RIFF "ACON") or a single .cur / .ico file: frames plus the steps that show them.
    class AnimatedCursor {
        public:
            /// @brief One animation step: which frame to show and for how long.
            struct Step {
                std::size_t frame = 0;
                int durationMs = 0;
            };

            /// @brief Reads and parses a cursor file.
            /// @param path File to read.
            /// @return The cursor, nothing when unreadable or malformed.
            static std::optional<AnimatedCursor> load(const std::filesystem::path& path);

            /// @brief Parses .ani data (RIFF "ACON" with "anih", optional "rate" and "seq " chunks and a "fram" list), or a single .cur / .ico.
            /// @param data File contents.
            /// @return The cursor, nothing when malformed.
            static std::optional<AnimatedCursor> parse(const core::Bytes& data);

            /// @brief Parses a single .cur / .ico image (the first one in the file).
            /// @param data File contents.
            /// @return The frame, nothing when malformed.
            static std::optional<CursorFrame> parseIcon(const core::Bytes& data);

            /// @brief Gives the images.
            /// @return The frames.
            const std::vector<CursorFrame>& frames() const { return m_frames; }

            /// @brief Gives the animation order and timing.
            /// @return The steps.
            const std::vector<Step>& steps() const { return m_steps; }

        private:
            std::vector<CursorFrame> m_frames;
            std::vector<Step> m_steps;
    };

}

#endif
