#ifndef NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_WINDOW_MEMORY_H
#define NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_WINDOW_MEMORY_H

#include <filesystem>
#include <optional>

namespace client {

    /// @brief The window as it was when the game last closed: its size when not maximized and whether it was maximized.
    /// @details user\qol-window.ini, from before extra-settings.json; now only read once by ExtraSettings to migrate it.
    struct WindowMemory {
        int width = 800; // the original's window width in pixels
        int height = 600; // the original's window height in pixels
        bool maximized = false;

        /// @brief Gives the path of the window file.
        /// @param userRoot The user folder.
        /// @return The path of qol-window.ini in it.
        static std::filesystem::path fileIn(const std::filesystem::path& userRoot);

        /// @brief Reads the window file.
        /// @param file The file.
        /// @return The window, or nothing when the file is not there or not readable.
        static std::optional<WindowMemory> load(const std::filesystem::path& file);

        /// @brief Writes the window file.
        /// @param file The file.
        void save(const std::filesystem::path& file) const;
    };

}

#endif
