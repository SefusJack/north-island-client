#ifndef NORTH_ISLAND_FORMATS_SOUND_TABLE_H
#define NORTH_ISLAND_FORMATS_SOUND_TABLE_H

#include "core/Bytes.h"

#include <optional>
#include <string>
#include <vector>

namespace formats {

    /// @brief The sound file names of sound\soundtabel.txt.
    /// @details Original: TForm1.LoadSoundTable; SoundFileTable[n] is the n-th line after the skipped first line. The ids
    /// of the movies' music and the paths' sounds are its places.
    class SoundTable {
        public:
            /// @brief Splits the text file into its file names (one a line, the first line skipped).
            /// @param data The raw file bytes.
            /// @return The table.
            static SoundTable parse(const core::Bytes& data);

            /// @brief The file name of a sound id.
            /// @param id The sound id, counted from 1.
            /// @return The file name, or nothing for an id past the table's end.
            std::optional<std::string> file(int id) const;

        private:
            std::vector<std::string> m_files;
    };

}

#endif
