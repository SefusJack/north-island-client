#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_LOG_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_FRAME_LOG_H

#include "client/services/frame/FrameStats.h"

#include <filesystem>
#include <fstream>

namespace client {

    /// @brief Writer of the --perf-log CSV file, one line per frame sample.
    class FrameLog {
        public:
            /// @brief Opens (truncates) the CSV file and writes its header.
            /// @param file Path of the CSV file.
            explicit FrameLog(
                const std::filesystem::path& file
            );

            /// @brief Tells whether the file could be opened.
            /// @return True when the file is open.
            bool isOpen() const { return static_cast<bool>(m_file); }

            /// @brief Appends one frame sample as a CSV line (gpu empty when not measured).
            /// @param sample The frame's timings.
            void add(const FrameSample& sample);

        private:
            std::ofstream m_file;
    };

}

#endif
