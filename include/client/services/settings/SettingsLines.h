#ifndef NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_SETTINGS_LINES_H
#define NORTH_ISLAND_CLIENT_SERVICES_SETTINGS_SETTINGS_LINES_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Reads a settings file as lines of "name=value" and section heads (the way the game reads it: TStringList.LoadFromFile), dropping a trailing CR.
    /// @param file The file.
    /// @return The lines (none when the file cannot be read).
    std::vector<std::string> readSettingsLines(const std::filesystem::path& file);

    /// @brief Writes the lines to a settings file, each ended with CR LF.
    /// @param file The file.
    /// @param lines The lines.
    void writeSettingsLines(const std::filesystem::path& file, const std::vector<std::string>& lines);

    /// @brief Finds the value of a name; the first line of the name counts (TStringList Values).
    /// @param lines The settings lines.
    /// @param name The setting's name.
    /// @return The text after "name=", or nothing when there is no such line.
    std::optional<std::string> settingValue(const std::vector<std::string>& lines, const std::string& name);

    /// @brief Reads the value of a name as a whole number.
    /// @param lines The settings lines.
    /// @param name The setting's name.
    /// @return The number, or nothing for an empty, missing or unreadable value.
    std::optional<int> settingInt(const std::vector<std::string>& lines, const std::string& name);

    /// @brief Sets the value of a name (Values[name] := value): the first such line, or a new one at the end.
    /// @param lines The settings lines.
    /// @param name The setting's name.
    /// @param value The new value.
    void setSettingValue(std::vector<std::string>& lines, const std::string& name, const std::string& value);

    /// @brief Tells whether a whole line (such as a section head) is in the lines.
    /// @param lines The settings lines.
    /// @param line The line to look for.
    /// @return True when found.
    bool hasSettingLine(const std::vector<std::string>& lines, const std::string& line);

}

#endif
