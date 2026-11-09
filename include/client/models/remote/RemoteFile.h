#ifndef NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_FILE_H
#define NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_FILE_H

#include "client/models/remote/AutoBattleSettings.h"
#include "client/models/remote/RemoteSettings.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The Remote Control's saved settings file.
    /// @details Original: User\sd<id xor 7654321>.asd, TTH_AutoPlayForm's SaveAutoPlayFile and LoadAutoPlayFile; lines of one length byte and
    /// the bytes xor 21: the version, the ticks, the seven values, the supply places, the five items and the two sets of the Plus form.
    struct RemoteFile {
        static constexpr int kVersion = 3006; // file version written and read
        static constexpr int kOlderVersion = 3005; // previous file version, still accepted

        int version = kVersion;
        RemoteSettings settings;
        std::array<AutoBattleSettings, 3> plusSets{}; // sets of the Plus form (the file holds two)

        /// @brief Builds the path of a player's Remote Control file (sd<id xor 7654321>.asd).
        /// @param userRoot Directory holding the per-player files.
        /// @param playerId Id of the player.
        /// @return The file path.
        static std::filesystem::path pathFor(const std::filesystem::path& userRoot, std::int32_t playerId);

        /// @brief Reads and decodes the file (original LoadAutoPlayFile).
        /// @param path File to read.
        /// @return The settings; nothing for a missing or short file; a file of another version comes back with its version only.
        static std::optional<RemoteFile> read(const std::filesystem::path& path);

        /// @brief Encodes and writes the file (original SaveAutoPlayFile).
        /// @param path File to write.
        /// @return True when written.
        bool write(const std::filesystem::path& path) const;
    };

}

#endif
