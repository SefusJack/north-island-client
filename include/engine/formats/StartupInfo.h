#ifndef NORTH_ISLAND_FORMATS_STARTUP_INFO_H
#define NORTH_ISLAND_FORMATS_STARTUP_INFO_H

#include <string>
#include <vector>

namespace formats {

    /// @brief The launcher's startup choices (Sinfo.txt): screen mode, script and memory mode.
    struct StartupInfo {
        /// @brief How much memory the client may use for its resources.
        enum class MemoryMode { Full, Half, Short };

        bool windowed = false;
        bool simplified = false;
        MemoryMode memory = MemoryMode::Full;

        /// @brief Reads the lines of Sinfo.txt written by the launcher.
        /// @param lines The text lines (screen mode, script, memory mode).
        /// @return The startup info.
        static StartupInfo parse(const std::vector<std::string>& lines);

        /// @brief Builds the info from the launcher's three command-line numbers.
        /// @details Original: aLogin.exe SCREEN LANGUAGE MEMORY (TForm1.FormCreate): screen 2 windowed, language 1
        /// Simplified, memory 0 full, 1 half, any other number short; one that is not a number counts as windowed,
        /// Traditional and half.
        /// @param screen The screen argument.
        /// @param language The language argument.
        /// @param memory The memory argument.
        /// @return The startup info.
        static StartupInfo fromArguments(const std::string& screen, const std::string& language, const std::string& memory);
    };

}

#endif
