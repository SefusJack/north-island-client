#include "client/services/settings/TalkSettings.h"

#include "client/services/settings/SettingsLines.h"

#include <string>
#include <vector>

namespace client {

    TalkSettings TalkSettings::load(const std::filesystem::path& file, std::uint32_t playerId) {
        const TalkSettings defaults;
        if (!std::filesystem::exists(file)) {
            return defaults;
        }
        const std::vector<std::string> lines = readSettingsLines(file);
        if (settingValue(lines, "User") != std::to_string(playerId) || !hasSettingLine(lines, "[Talk]")) {
            return defaults;
        }
        const auto mode = settingInt(lines, "FormMode");
        const auto x = settingInt(lines, "FormPositionX");
        const auto y = settingInt(lines, "FormPositionY");
        const auto width = settingInt(lines, "FormSizeW");
        const auto height = settingInt(lines, "FormSizeH");
        const auto background = settingInt(lines, "BackGroundNo");
        const auto channel = settingInt(lines, "MsgChannel");
        const auto resizeX = settingInt(lines, "ReSizeX");
        const auto resizeY = settingInt(lines, "ReSizeY");
        if (!mode || !x || !y || !width || !height || !background || !channel || !resizeX || !resizeY) {
            return defaults;
        }
        TalkSettings read;
        read.formMode = *mode;
        read.form = {*x, *y, *width, *height};
        read.background = *background;
        read.resizeButton = {*resizeX, *resizeY};
        return read;
    }

    void TalkSettings::save(const std::filesystem::path& file, std::uint32_t playerId, int chatChannel) const {
        std::vector<std::string> lines;
        if (std::filesystem::exists(file)) {
            lines = readSettingsLines(file);
            if (!hasSettingLine(lines, "[Talk]")) {
                lines.push_back("[Talk]");
            }
            setSettingValue(lines, "User", std::to_string(playerId));
            const TalkSettings defaults;
            const auto put = [&lines](const char* name, int value, int fallback) {
                setSettingValue(lines, name, std::to_string(settingValue(lines, name) ? value : fallback));
            };
            put("FormMode", formMode, defaults.formMode);
            put("FormPositionX", form.x, defaults.form.x);
            put("FormPositionY", form.y, defaults.form.y);
            put("FormSizeW", form.width, defaults.form.width);
            put("FormSizeH", form.height, defaults.form.height);
            put("BackGroundNo", background, defaults.background);
            put("MsgChannel", chatChannel, 2); // fallback chat channel 2
            put("ReSizeX", resizeButton.x, defaults.resizeButton.x);
            put("ReSizeY", resizeButton.y, defaults.resizeButton.y);
        }
        writeSettingsLines(file, lines);
    }

}
