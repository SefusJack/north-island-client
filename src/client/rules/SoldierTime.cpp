#include "client/rules/SoldierTime.h"

namespace client {

    std::string soldierTimeText(int minutes) {
        constexpr int kMinutesInHour = 60; // minutes per hour
        constexpr int kHoursInDay = 24; // hours per day
        const int hours = minutes / kMinutesInHour;
        const int days = hours / kHoursInDay;
        const int rest = minutes % kMinutesInHour;
        std::string text;
        if (days != 0) {
            text += std::to_string(days) + " Day ";
        }
        if (hours != 0) {
            text += std::to_string(hours) + " Hour ";
        }
        if (rest != 0) {
            text += text.empty() ? std::to_string(rest) + " Minute " : "Again " + std::to_string(rest) + " Minute ";
        }
        return text;
    }

}
