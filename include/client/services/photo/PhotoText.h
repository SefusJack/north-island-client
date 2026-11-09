#ifndef NORTH_ISLAND_CLIENT_SERVICES_PHOTO_PHOTO_TEXT_H
#define NORTH_ISLAND_CLIENT_SERVICES_PHOTO_PHOTO_TEXT_H

#include "formats/Image.h"

#include <ctime>
#include <string>

namespace client {

    /// @brief The two lines drawn on a photo with the operating system's text (standard font in black on white) and where they come from.
    /// @details Original: TForm1.GetPicAndSave2, TextOut of the bitmap's canvas.
    class PhotoText {
        public:
            /// @brief Formats the date line: the weekday and month by name, the day, the year, the 12-hour clock.
            /// @details Original: FormatDateTime('dddd, mmmm d"<Big5 day>", yyyy, hh:mm:ss AM/PM').
            /// @param now The time to format.
            /// @return The date line.
            static std::string dateLine(std::time_t now);

            /// @brief Finds the last address the host's name gives.
            /// @details Original: TForm1.GetIP.
            /// @return The address, or "" when none.
            static std::string localAddress();

            /// @brief Writes the date line at 0, 0 and the address line at 0, 15 on the picture (only where the system's own text drawing is: nothing elsewhere).
            /// @param picture The picture to write on.
            /// @param date The date line.
            /// @param address The address line.
            static void stamp(formats::Image& picture, const std::string& date, const std::string& address);
    };

}

#endif
