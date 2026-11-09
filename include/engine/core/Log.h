#ifndef NORTH_ISLAND_CORE_LOG_H
#define NORTH_ISLAND_CORE_LOG_H

#include <iostream>
#include <sstream>

namespace core {

    /// @brief Writes one line "[level] parts..." to std::clog.
    /// @tparam Parts Types of the streamed parts.
    /// @param level Level tag.
    /// @param parts Values streamed after the tag.
    template <typename... Parts>
    void logLine(const char* level, const Parts&... parts) {
        std::ostringstream line;
        line << '[' << level << "] ";
        (line << ... << parts);
        line << '\n';
        std::clog << line.str() << std::flush;
    }

    /// @brief Logs an info line.
    /// @tparam Parts Types of the streamed parts.
    /// @param parts Values to log.
    template <typename... Parts>
    void logInfo(const Parts&... parts) {
        logLine("info", parts...);
    }

    /// @brief Logs a warning line.
    /// @tparam Parts Types of the streamed parts.
    /// @param parts Values to log.
    template <typename... Parts>
    void logWarning(const Parts&... parts) {
        logLine("warn", parts...);
    }

    /// @brief Logs an error line.
    /// @tparam Parts Types of the streamed parts.
    /// @param parts Values to log.
    template <typename... Parts>
    void logError(const Parts&... parts) {
        logLine("error", parts...);
    }

}

#endif
