#ifndef NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_SYSTEM_SETTINGS_H
#define NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_SYSTEM_SETTINGS_H

#include <array>
#include <cstdint>
#include <filesystem>

namespace client {

    /// @brief What the system window keeps per character: music and sound on or off and their volumes 0..10, the chat colours of the local, whisper, team, guild and world channels (ChatColorTable indexes), the camera switch and the character information switches.
    /// @details Original: user\save<id xor 121>.dat, LoadSystemSettings / SaveSystemSettings.
    struct SystemSettings {
        bool musicOn = true;
        bool soundOn = true;
        int musicVolume = 5; // default music volume (0..10)
        int soundVolume = 5; // default sound volume (0..10)
        std::array<int, 5> channelColors{9, 10, 6, 4, 1}; // default ChatColorTable index for local, whisper, team, guild, world
        int battleSwitch = 1;
        std::array<int, 9> humInfo{1, 1, 1, 1, 1, 1, 1, 1, 0}; // the 9 character information switches (HumInfo1..9): all on but the last

        /// @brief Gives the character's settings file.
        /// @param userRoot The user folder.
        /// @param playerId The character's id.
        /// @return The path of save<id xor 121>.dat in the user folder.
        static std::filesystem::path fileFor(const std::filesystem::path& userRoot, std::uint32_t playerId);

        /// @brief Reads the [System] settings; every key must be there, anything missing gives the defaults.
        /// @param file The character's settings file.
        /// @param playerId The character's id.
        /// @return The settings.
        static SystemSettings load(const std::filesystem::path& file, std::uint32_t playerId);

        /// @brief Saves the [System] settings as the game does: only a file that is already there gets the values; a new one is written empty.
        /// @param file The character's settings file.
        /// @param playerId The character's id.
        void save(const std::filesystem::path& file, std::uint32_t playerId) const;
    };

    /// @brief Looks up a chat colour in ChatColorTable (InitChatColors with GetPresetColor16).
    /// @param index The colour's index.
    /// @return The colour as RGB565 (0 for an index out of range).
    std::uint16_t chatColor(int index);

}

#endif
