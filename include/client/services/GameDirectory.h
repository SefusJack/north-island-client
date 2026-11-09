#ifndef NORTH_ISLAND_CLIENT_SERVICES_GAME_DIRECTORY_H
#define NORTH_ISLAND_CLIENT_SERVICES_GAME_DIRECTORY_H

#include "ui/LayoutSource.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief Where things are: the read-only game install, the writable user folder and the screen layouts.
    class GameDirectory {
        public:
            /// @brief Creates the directory set.
            /// @param data The game install folder.
            /// @param user The folder the client writes to (settings, account list, log).
            /// @param layouts A folder whose files are read before the compiled-in layouts; empty for none.
            GameDirectory(
                std::filesystem::path data,
                std::filesystem::path user,
                std::filesystem::path layouts = {}
            )
                : m_data(std::move(data)),
                  m_user(std::move(user)),
                  m_layouts(std::move(layouts)) {}

            /// @brief Resolves a file of the game install, named the way the game names it; a sound a mod pack has is the pack's.
            /// @param relative Game-style relative name, e.g. "Menu\Skins\White\x.jpg".
            /// @return The full path.
            std::filesystem::path data(const std::string& relative) const;

            /// @brief Sets the lookup for the mod packs' sounds (ModPacks::sound).
            /// @param sound Returns the pack's file for a game-style name, or nothing.
            void setSoundPacks(std::function<std::optional<std::filesystem::path>(const std::string&)> sound) { m_soundPacks = std::move(sound); }

            /// @brief Resolves a file in the writable user folder.
            /// @param relative Name relative to the user folder.
            /// @return The full path.
            std::filesystem::path user(const std::string& relative) const { return m_user / relative; }

            /// @brief Loads a screen layout by bare file name; throws ui::LayoutError when there is none.
            /// @param fileName Bare layout file name, e.g. "LoginForm.xml".
            /// @return The layout source.
            ui::LayoutSource layout(const std::string& fileName) const;

            /// @brief The writable user folder itself.
            /// @return The user folder path.
            const std::filesystem::path& userRoot() const { return m_user; }

        private:
            std::filesystem::path m_data;
            std::filesystem::path m_user;
            std::filesystem::path m_layouts;
            std::function<std::optional<std::filesystem::path>(const std::string&)> m_soundPacks;
    };

}

#endif
