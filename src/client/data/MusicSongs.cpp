#include "client/data/MusicSongs.h"

#include <algorithm>
#include <cctype>

namespace client {

    namespace {

        /// @brief Upper-cases a text (ASCII, per byte).
        /// @param text The text, taken by value.
        /// @return The upper-cased text.
        std::string upper(std::string text) {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
            return text;
        }

    }

    void MusicSongs::scan() {
        std::vector<std::string> files;
        std::error_code error;
        for (const auto& entry : std::filesystem::directory_iterator(m_folder, error)) {
            if (!entry.is_regular_file(error) || upper(entry.path().extension().string()) != ".WAV") {
                continue;
            }
            const std::string stem = entry.path().stem().string();
            const std::string head = upper(stem.substr(0, 3)); // first 3 characters: the SEB / WAV prefixes
            if (head == "SEB" || head == "WAV" || upper(stem) == "BGM0011" || upper(stem) == "9911") {
                continue;
            }
            files.push_back(stem);
        }
        std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) { return upper(a) < upper(b); });
        m_names.assign({std::string(), "BGM0011"});
        m_names.insert(m_names.end(), files.begin(), files.end());
    }

    std::string MusicSongs::name(int number) const { return number >= 1 && number <= count() ? m_names[static_cast<std::size_t>(number)] : std::string(); }

    int MusicSongs::find(const std::string& name) const {
        for (int number = 1; number <= count(); ++number) {
            if (upper(m_names[static_cast<std::size_t>(number)]) == upper(name)) {
                return number;
            }
        }
        return 0;
    }

}
