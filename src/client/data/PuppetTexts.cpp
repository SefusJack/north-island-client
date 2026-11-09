#include "client/data/PuppetTexts.h"

namespace client {

    std::string puppetDuration(int minutes) {
        const int days = (minutes / 1440) & 0xFF; // 1440 minutes per day; the original keeps the days in a Byte (mask 0xFF)
        const int hours = minutes % 1440 / 60; // 60 minutes per hour
        const int rest = minutes % 1440 % 60; // minutes left after whole days and hours
        if (days > 0) {
            return std::to_string(days) + " Day " + std::to_string(hours) + " Hour " + std::to_string(rest) + " Minute ";
        }
        if (hours > 0) {
            return std::to_string(hours) + " Hour " + std::to_string(rest) + " Minute ";
        }
        return std::to_string(rest) + " Minute ";
    }

}
