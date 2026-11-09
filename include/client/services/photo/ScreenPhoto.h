#ifndef NORTH_ISLAND_CLIENT_SERVICES_PHOTO_SCREEN_PHOTO_H
#define NORTH_ISLAND_CLIENT_SERVICES_PHOTO_SCREEN_PHOTO_H

#include "client/services/photo/PhotoCheck.h"
#include "formats/Image.h"

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace client {

    /// @brief The screenshot of Alt+P, saved in the photo folder of the game as mmddhhnnss.bmp (month, day, hour, minute, second).
    /// @details Original: TForm1.GetPicAndSave2.
    class ScreenPhoto {
        public:
            /// @brief Picks the first free name for a photo; when the file exists the game waits for the clock to move on, so the name takes the next second.
            /// @param photoFolder The photo folder.
            /// @param now The time the photo is taken at.
            /// @return The path of the .bmp file.
            static std::filesystem::path chooseFile(const std::filesystem::path& photoFolder, std::time_t now);

            /// @brief Builds the message the game shows once the photo is saved ("Have saved in <path>").
            /// @param file The saved file.
            /// @return The message.
            static std::string savedMessage(const std::filesystem::path& file);

            /// @brief Does the rest of GetPicAndSave2 on the screen as drawn: writes the date and the host's address on the picture, saves the .bmp (24 bit, rows from the bottom) and next to it the .bft check file.
            /// @param bmpFile The .bmp file to write.
            /// @param screen The screen picture (taken by value; the text is written on the copy).
            /// @param check The world the photo was taken in.
            /// @param now The time of the photo.
            /// @return False when a file cannot be written.
            static bool save(const std::filesystem::path& bmpFile, formats::Image screen, const PhotoCheck& check, std::time_t now);

            /// @brief Builds the check file's bytes: 18096 bytes xor 39 holding the player, its map and place, the host's address, the name of the photo, the sums of the red, green and blue bytes, the sum of the green bytes of 12 rows and of the blue bytes of 12 others (each as the pixel's parity mark would leave it: every other pixel has its byte even, the rest odd) and the players of the scene.
            /// @details Original: ft_CheckBmp.SaveDecodedBmp's file.
            /// @param picture The photo's picture.
            /// @param check The world the photo was taken in.
            /// @param address The host's address.
            /// @param name The name of the photo, without its extension.
            /// @return The encoded file contents.
            static std::vector<std::uint8_t> checkFile(
                const formats::Image& picture,
                const PhotoCheck& check,
                const std::string& address,
                const std::string& name
            );
    };

}

#endif
