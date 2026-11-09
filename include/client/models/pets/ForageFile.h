#ifndef NORTH_ISLAND_CLIENT_MODELS_PETS_FORAGE_FILE_H
#define NORTH_ISLAND_CLIENT_MODELS_PETS_FORAGE_FILE_H

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>

namespace client {

    /// @brief The items the Dog Bone's form was last set to fetch, stored in the per-player Forage file.
    /// @details Original: User\<id>Forage, MK_Menu.pas SaveForageSettings and LoadForageSettings; four lines (the first
    /// is the unused target 0), each a length byte and the number's text xor 21.
    struct ForageFile {
        std::array<int, 4> items{};

        /// @brief Builds the path of a player's Forage file.
        /// @param userRoot Directory holding the per-player files.
        /// @param playerId Id of the player.
        /// @return The path of the player's Forage file.
        static std::filesystem::path pathFor(const std::filesystem::path& userRoot, std::int32_t playerId);

        /// @brief Reads and decodes a Forage file (original LoadForageSettings).
        /// @param path File to read.
        /// @return The settings, or nothing for a missing, short or malformed file.
        static std::optional<ForageFile> read(const std::filesystem::path& path);

        /// @brief Encodes and writes the settings (original SaveForageSettings).
        /// @param path File to write.
        /// @return True when the file was written.
        bool write(const std::filesystem::path& path) const;
    };

}

#endif
