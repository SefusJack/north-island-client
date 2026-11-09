#include "client/services/settings/SystemSettings.h"

#include "client/services/settings/SettingsLines.h"

#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr std::uint32_t kFileKey = 121; // XORed with the player id to form the "save<N>.dat" file number

    }

    std::filesystem::path SystemSettings::fileFor(const std::filesystem::path& userRoot, std::uint32_t playerId) {
        return userRoot / ("save" + std::to_string(playerId ^ kFileKey) + ".dat");
    }

    SystemSettings SystemSettings::load(const std::filesystem::path& file, std::uint32_t playerId) {
        SystemSettings settings;
        const std::vector<std::string> lines = readSettingsLines(file);
        if (settingValue(lines, "User") != std::to_string(playerId) || !hasSettingLine(lines, "[System]")) {
            return settings;
        }
        SystemSettings read;
        const auto music = settingInt(lines, "musicOn");
        const auto sound = settingInt(lines, "soundOn");
        const auto musicVolume = settingInt(lines, "musicVolume");
        const auto soundVolume = settingInt(lines, "soundVolume");
        if (!music || !sound || !musicVolume || !soundVolume) {
            return settings;
        }
        read.musicOn = *music == 1;
        read.soundOn = *sound == 1;
        read.musicVolume = *musicVolume;
        read.soundVolume = *soundVolume;
        for (std::size_t i = 0; i < read.humInfo.size(); ++i) {
            const auto value = settingInt(lines, "HumInfo" + std::to_string(i + 1));
            if (!value) {
                return settings;
            }
            read.humInfo[i] = *value > 0 ? 1 : 0;
        }
        for (std::size_t i = 0; i < read.channelColors.size(); ++i) {
            const auto value = settingInt(lines, "ChannelColor" + std::to_string(i + 1));
            if (!value) {
                return settings;
            }
            read.channelColors[i] = *value;
        }
        const auto battle = settingInt(lines, "BattleSwitch");
        if (!battle) {
            return settings;
        }
        read.battleSwitch = *battle;
        return read;
    }

    void SystemSettings::save(const std::filesystem::path& file, std::uint32_t playerId) const {
        std::vector<std::string> lines;
        if (std::filesystem::exists(file)) {
            lines = readSettingsLines(file);
            setSettingValue(lines, "User", std::to_string(playerId));
            if (!hasSettingLine(lines, "[System]")) {
                lines.push_back("[System]");
            }
            setSettingValue(lines, "musicOn", musicOn ? "1" : "2"); // 1 on, 2 off
            setSettingValue(lines, "soundOn", soundOn ? "1" : "2"); // 1 on, 2 off
            setSettingValue(lines, "musicVolume", std::to_string(musicVolume));
            setSettingValue(lines, "soundVolume", std::to_string(soundVolume));
            for (std::size_t i = 0; i < humInfo.size(); ++i) {
                setSettingValue(lines, "HumInfo" + std::to_string(i + 1), std::to_string(humInfo[i] != 0 ? 1 : 0));
            }
            for (std::size_t i = 0; i < channelColors.size(); ++i) {
                setSettingValue(lines, "ChannelColor" + std::to_string(i + 1), std::to_string(channelColors[i]));
            }
            setSettingValue(lines, "BattleSwitch", std::to_string(battleSwitch));
        }
        writeSettingsLines(file, lines);
    }

    std::uint16_t chatColor(int index) {
        // the 12 preset chat colours as RGB565
        static const std::array<std::uint16_t, 12> kColors = {0xFE31, 0xFFB3, 0xC6F3, 0x8653, 0x6E7E, 0x8E27, 0xFFFF, 0xF800, 0xF4A3, 0xFF80, 0xFC00, 0x0000};
        return index >= 0 && index < static_cast<int>(kColors.size()) ? kColors[static_cast<std::size_t>(index)] : 0;
    }

}
