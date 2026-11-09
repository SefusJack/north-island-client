#include "formats/SoundTable.h"

namespace formats {

    SoundTable SoundTable::parse(const core::Bytes& data) {
        SoundTable table;
        std::string line;
        bool first = true;
        const auto finishLine = [&] {
            if (!first) {
                table.m_files.push_back(line);
            }
            first = false;
            line.clear();
        };
        for (const std::uint8_t byte : data) {
            if (byte == '\n') {
                finishLine();
            } else if (byte != '\r') {
                line.push_back(static_cast<char>(byte));
            }
        }
        if (!line.empty()) {
            finishLine();
        }
        return table;
    }

    std::optional<std::string> SoundTable::file(int id) const {
        if (id < 1 || static_cast<std::size_t>(id) > m_files.size()) {
            return std::nullopt;
        }
        return m_files[static_cast<std::size_t>(id) - 1];
    }

}
