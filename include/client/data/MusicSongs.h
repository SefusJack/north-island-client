#ifndef NORTH_ISLAND_CLIENT_DATA_MUSIC_SONGS_H
#define NORTH_ISLAND_CLIENT_DATA_MUSIC_SONGS_H

#include <filesystem>
#include <string>
#include <vector>

namespace client {

    /// @brief The songs the Music Center offers.
    /// @details Original: TMusicCenter.FMusicFiles, RE_MigrantManage_Tools.pas. Number 1 is "BGM0011" (shown as "Set music"), then the .wav files of the game's sound folder in name order, without the sound effects (names starting SEB or WAV), BGM0011 and 9911, as ScanMusicFiles lists them.
    class MusicSongs {
        public:
            /// @brief Creates the list and scans the folder.
            /// @param soundFolder The game's sound folder.
            explicit MusicSongs(
                std::filesystem::path soundFolder
            )
                : m_folder(std::move(soundFolder)) {
                scan();
            }

            /// @brief Reads the folder again (Original: ScanMusicFiles).
            void scan();

            /// @brief Gives the number of songs (Original: FSongCount).
            /// @return The song count.
            int count() const { return static_cast<int>(m_names.size()) - 1; }

            /// @brief Gives the song's file name without .wav (Original: FMusicFiles[number]).
            /// @param number Song number (1..count).
            /// @return The name, or an empty string outside 1..count.
            std::string name(int number) const;

            /// @brief Finds the number of a song by name, ignoring case (Original: FindMusicIndex).
            /// @param name The song's file name without .wav.
            /// @return The song number, or 0 for none.
            int find(const std::string& name) const;

        private:
            std::filesystem::path m_folder;
            std::vector<std::string> m_names;
    };

}

#endif
