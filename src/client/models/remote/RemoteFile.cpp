#include "client/models/remote/RemoteFile.h"

#include "core/FileSystem.h"

#include <algorithm>
#include <array>
#include <functional>

namespace client {

    namespace {

        constexpr std::uint8_t kKey = 21; // xor key of the file's bytes
        constexpr std::int32_t kIdKey = 7654321; // xor key of the player id in the file name
        constexpr std::size_t kFirstPlusLine = 16; // index of the first auto-battle ("plus") settings line
        constexpr std::size_t kPlusLines = 56; // number of lines of the two plus settings sets together

        /// @brief Encodes a 16-bit word as two bytes, low first (original WordToBinStr).
        /// @param value The word.
        /// @return The 2-byte text.
        std::string wordText(int value) {
            std::string text;
            text += static_cast<char>(value & 0xFF); // low byte mask
            text += static_cast<char>((value >> 8) & 0xFF); // high byte: shift 8 bits, byte mask
            return text;
        }

        /// @brief Encodes a 32-bit integer as four bytes, low first (original IntToBinStr).
        /// @param value The integer.
        /// @return The 4-byte text.
        std::string intText(std::int32_t value) {
            std::string text;
            for (int shift = 0; shift < 32; shift += 8) { // 32 bits, one byte (8 bits) at a time
                text += static_cast<char>((static_cast<std::uint32_t>(value) >> shift) & 0xFF); // byte mask
            }
            return text;
        }

        /// @brief Decodes a 16-bit word from the first two bytes, low first (original BinStrToWord).
        /// @param text The bytes.
        /// @return The word, 0 when the text is shorter than 2 bytes.
        int textWord(const std::string& text) {
            return text.size() < 2 ? 0 : static_cast<unsigned char>(text[0]) | (static_cast<unsigned char>(text[1]) << 8); // 2 bytes; high byte shifted 8 bits
        }

        /// @brief Decodes a 32-bit integer from the first four bytes, low first (original BinStrToInt).
        /// @param text The bytes.
        /// @return The integer, 0 when the text is shorter than 4 bytes.
        std::int32_t textInt(const std::string& text) {
            if (text.size() < 4) { // an integer is 4 bytes
                return 0;
            }
            std::uint32_t value = 0;
            for (std::size_t i = 0; i < 4; ++i) { // 4 bytes
                value |= static_cast<std::uint32_t>(static_cast<unsigned char>(text[i])) << (8 * i); // 8 bits per byte
            }
            return static_cast<std::int32_t>(value);
        }

    }

    std::filesystem::path RemoteFile::pathFor(const std::filesystem::path& userRoot, std::int32_t playerId) {
        return userRoot / ("sd" + std::to_string(playerId ^ kIdKey) + ".asd");
    }

    std::optional<RemoteFile> RemoteFile::read(const std::filesystem::path& path) {
        const auto bytes = core::readFile(path);
        if (!bytes) {
            return std::nullopt;
        }
        std::vector<std::string> lines;
        std::size_t at = 0;
        while (at < bytes->size()) {
            const std::size_t length = (*bytes)[at++];
            std::string line;
            for (std::size_t i = 0; i < length && at < bytes->size(); ++i) {
                line += static_cast<char>((*bytes)[at++] ^ kKey);
            }
            lines.push_back(line);
        }
        if (lines.empty()) {
            return std::nullopt;
        }
        RemoteFile file;
        file.version = textWord(lines[0]);
        if (file.version != kVersion && file.version != kOlderVersion) {
            return file;
        }
        if (lines.size() < kFirstPlusLine + kPlusLines) {
            return std::nullopt;
        }
        const int ticks = textWord(lines[1]);
        for (int tick = 1; tick <= RemoteSettings::kTicks; ++tick) {
            file.settings.ticks[static_cast<std::size_t>(tick)] = ((ticks >> (tick - 1)) & 1) != 0;
        }
        // lines 2..10: HP%, SP%, pet HP%, pet SP%, death limit, pet death limit, leave minutes, item from, item to
        file.settings.hpPercent = textInt(lines[2]);
        file.settings.spPercent = textInt(lines[3]);
        file.settings.petHpPercent = textInt(lines[4]);
        file.settings.petSpPercent = textInt(lines[5]);
        file.settings.deathLimit = textInt(lines[6]);
        file.settings.petDeathLimit = textInt(lines[7]);
        file.settings.leaveMinutes = textInt(lines[8]);
        file.settings.itemFrom = textInt(lines[9]);
        file.settings.itemTo = textInt(lines[10]);
        for (std::size_t item = 1; item <= RemoteSettings::kKeepItems; ++item) {
            file.settings.keepItems[item] = textInt(lines[10 + item]); // the keep items follow line 10
        }
        std::size_t line = kFirstPlusLine;
        for (std::size_t set = 1; set <= 2; ++set) {
            AutoBattleSettings& sets = file.plusSets[set];
            const int bits = textWord(lines[line++]);
            for (std::size_t flag = 1; flag <= 6; ++flag) { // the six options (flags 1..6)
                sets.flags[flag] = static_cast<std::uint8_t>((bits >> (flag - 1)) & 1);
            }
            for (std::size_t i = 1; i <= 2; ++i) { // two default skills
                sets.defaultSkillIds[i] = textInt(lines[line++]);
            }
            using Row = std::array<int, 4>;
            // seven rows of three entries (1..3) each
            for (Row& values : std::array<std::reference_wrapper<Row>, 7>{
                     sets.procSkillIds,
                     sets.procSkillIsItem,
                     sets.procSkillTargets,
                     sets.healThresholds,
                     sets.healIsItem,
                     sets.healActionIds,
                     sets.healTargets
                 }) {
                for (std::size_t i = 1; i <= 3; ++i) {
                    values[i] = textInt(lines[line++]);
                }
            }
            sets.faintCompareMode = textInt(lines[line++]);
            sets.faintHpValue = textInt(lines[line++]);
            sets.faintActionId = textInt(lines[line++]);
            sets.faintTarget = textInt(lines[line++]);
        }
        return file;
    }

    bool RemoteFile::write(const std::filesystem::path& path) const {
        std::vector<std::string> lines;
        lines.push_back(wordText(version));
        int ticks = 0;
        for (int tick = 1; tick <= RemoteSettings::kTicks; ++tick) {
            if (settings.ticks[static_cast<std::size_t>(tick)]) {
                ticks |= 1 << (tick - 1);
            }
        }
        lines.push_back(wordText(ticks));
        for (const int value :
             {settings.hpPercent,
              settings.spPercent,
              settings.petHpPercent,
              settings.petSpPercent,
              settings.deathLimit,
              settings.petDeathLimit,
              settings.leaveMinutes,
              settings.itemFrom,
              settings.itemTo}) {
            lines.push_back(intText(value));
        }
        for (std::size_t item = 1; item <= RemoteSettings::kKeepItems; ++item) {
            lines.push_back(intText(settings.keepItems[item]));
        }
        for (std::size_t set = 1; set <= 2; ++set) {
            const AutoBattleSettings& sets = plusSets[set];
            int bits = 0;
            for (std::size_t flag = 1; flag <= 6; ++flag) { // the six options (flags 1..6)
                if (sets.flags[flag] != 0) {
                    bits |= 1 << (flag - 1);
                }
            }
            lines.push_back(wordText(bits));
            for (std::size_t i = 1; i <= 2; ++i) { // two default skills
                lines.push_back(intText(sets.defaultSkillIds[i]));
            }
            using Row = std::array<int, 4>;
            // seven rows of three entries (1..3) each
            for (const Row& values : std::array<std::reference_wrapper<const Row>, 7>{
                     sets.procSkillIds,
                     sets.procSkillIsItem,
                     sets.procSkillTargets,
                     sets.healThresholds,
                     sets.healIsItem,
                     sets.healActionIds,
                     sets.healTargets
                 }) {
                for (std::size_t i = 1; i <= 3; ++i) {
                    lines.push_back(intText(values[i]));
                }
            }
            lines.push_back(intText(sets.faintCompareMode));
            lines.push_back(intText(sets.faintHpValue));
            lines.push_back(intText(sets.faintActionId));
            lines.push_back(intText(sets.faintTarget));
        }
        core::Bytes data;
        for (const std::string& line : lines) {
            const std::string stored = line.empty() ? std::string(1, '\r') : line;
            const std::size_t length = std::min<std::size_t>(stored.size(), 255); // longest line a length byte can hold
            data.push_back(static_cast<std::uint8_t>(length));
            for (std::size_t i = 0; i < length; ++i) {
                data.push_back(static_cast<std::uint8_t>(static_cast<std::uint8_t>(stored[i]) ^ kKey));
            }
        }
        return core::writeFile(path, data);
    }

}
