#include "formats/ServerList.h"

#include "core/StringUtil.h"

#include <array>

namespace formats {

    namespace {

        /// @brief Tells whether a line starts a group ("NN[" prefix).
        /// @param line The INI line.
        /// @return True for a group header line.
        bool isGroupHeader(const std::string& line) { return line.size() >= 3 && line[2] == '['; } // header is "NN[": '[' sits at index 2

        /// @brief A colour tag found in a text and where it starts.
        struct ColorTag {
            core::Color color;
            std::size_t position = 0;
        };

        /// @brief Finds a "<R>", "<G>" or "<B>" tag anywhere in the text, as the 16-bit colours the game uses.
        /// @param text The text to search.
        /// @return The colour and the tag's position, or nothing when there is no such tag.
        std::optional<ColorTag> findColorTag(const std::string& text) {
            const std::size_t lt = text.find('<');
            const std::size_t gt = text.find('>');
            if (lt == std::string::npos || gt == std::string::npos || gt != lt + 2) { // tag is exactly one letter wide: "<X>"
                return std::nullopt;
            }
            switch (text[lt + 1]) {
                case 'R': // red tag
                    return ColorTag{core::Color::fromRgb565(0xF800), lt}; // RGB565 red
                case 'G': // green tag
                    return ColorTag{core::Color::fromRgb565(0x0540), lt}; // RGB565 green
                case 'B': // blue tag
                    return ColorTag{core::Color::fromRgb565(0x001F), lt}; // RGB565 blue
                default: // any other letter: no colour tag
                    return std::nullopt;
            }
        }

    }

    ServerList ServerList::parse(const std::vector<std::string>& lines) {
        std::array<int, 100> headerLine{}; // one slot per group number 0..99
        headerLine.fill(-1);
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (!isGroupHeader(lines[i])) {
                continue;
            }
            const auto number = core::parseInt(lines[i].substr(0, 2)); // the two-digit group number
            if (number && *number > 0 && *number <= 99) { // highest group number
                headerLine[static_cast<std::size_t>(*number)] = static_cast<int>(i);
            }
        }

        ServerList list;
        for (int number = 1; number <= 99; ++number) { // highest group number
            const int lineIndex = headerLine[static_cast<std::size_t>(number)];
            if (lineIndex < 0) {
                continue;
            }
            const std::string& header = lines[static_cast<std::size_t>(lineIndex)];
            ServerGroup group;
            group.number = number;

            std::size_t close = header.find(']');
            if (close == std::string::npos) {
                close = header.size() - 1;
            }
            group.name = close > 3 ? header.substr(3, close - 3) : "[]"; // the name starts after the "NN[" prefix
            std::size_t codeStart = close + 1;
            if (header.back() != ']') {
                if (const auto tag = findColorTag(header)) {
                    group.color = tag->color;
                    codeStart += 3; // skip the "<C>" colour tag
                }
                group.code = core::parseInt(codeStart < header.size() ? header.substr(codeStart) : std::string()).value_or(0);
            }

            // at most 100 server lines follow a group header (header + 101 bound)
            for (std::size_t i = static_cast<std::size_t>(lineIndex) + 1; i < lines.size() && i <= static_cast<std::size_t>(lineIndex) + 101; ++i) {
                const std::string& line = lines[i];
                if (line.empty()) {
                    continue;
                }
                if (isGroupHeader(line)) {
                    break;
                }
                const std::size_t star = line.find('*');
                ServerEntry server;
                server.name = star == std::string::npos || star == 0 ? "No Name" : line.substr(0, star);
                if (star == std::string::npos || star + 1 >= line.size()) {
                    continue;
                }
                server.address = line.substr(star + 1);
                if (const auto tag = findColorTag(server.name)) {
                    server.color = tag->color;
                    server.name = server.name.substr(0, tag->position);
                }
                server.id = group.code * 100 + static_cast<int>(group.servers.size()) + 1; // server id: group code * 100 + 1-based index
                group.servers.push_back(server);
            }

            if (number == kPvpGroup) {
                if (!group.servers.empty()) {
                    list.m_pvpServer = group.servers.front();
                    list.m_pvpServer->name = group.name;
                }
            } else {
                list.m_groups.push_back(std::move(group));
            }
        }
        return list;
    }

}
