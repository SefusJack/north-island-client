#ifndef NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_TALK_SETTINGS_H
#define NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_TALK_SETTINGS_H

#include "core/Point.h"
#include "core/Rect.h"

#include <cstdint>
#include <filesystem>

namespace client {

    /// @brief The chat box as the character left it: its look (0 dithered, 1 see-through, 2 hidden), place and size, background and the resize button's place, from which the size is worked out again when the box is set up.
    /// @details Original: the [Talk] part of user\save<id xor 121>.dat, LoadTalkFormSettings / SaveTalkFormSettings.
    struct TalkSettings {
        int formMode = 1;
        core::Rect form{0, 475, 520, 100}; // default box place and size: x, y, width, height
        int background = 0;
        core::Point resizeButton{496, 2}; // default place of the resize button in the box

        /// @brief Reads the [Talk] settings; every key must be there, else all are the defaults.
        /// @param file The character's settings file.
        /// @param playerId The character's id.
        /// @return The settings.
        static TalkSettings load(const std::filesystem::path& file, std::uint32_t playerId);

        /// @brief Saves the [Talk] settings as the game does: only a file that is already there gets the values (a key missing so far gets its default); a new one is written empty.
        /// @param file The character's settings file.
        /// @param playerId The character's id.
        /// @param chatChannel The input bar's channel.
        void save(const std::filesystem::path& file, std::uint32_t playerId, int chatChannel) const;
    };

}

#endif
