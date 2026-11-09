#include "client/rules/SunSigns.h"

namespace client {

    int sunSign(int year, int month, int day) {
        if (year == 0 || month == 0 || day == 0) {
            return 0;
        }
        // start month and start day of each of the 12 signs (sign 1 starts March 21, sign 12 starts February 20)
        constexpr int kStartMonths[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 1, 2}; // sign 1..12 start month, 3 = March
        constexpr int kStartDays[] = {21, 21, 21, 22, 23, 23, 23, 23, 23, 23, 21, 20}; // sign 1..12 start day of that month
        int sign = 0;
        for (int i = 0; i < 12; ++i) { // 12 signs
            const int next = (i + 1) % 12; // 12 signs, wraps Pisces to Aries
            if ((month == kStartMonths[i] && day >= kStartDays[i]) || (month == kStartMonths[next] && day < kStartDays[next])) {
                sign = i + 1;
            }
        }
        return sign;
    }

    int heartRadarNumber(
        int birthYear,
        int birthMonth,
        int birthDay,
        int sunSign,
        int profileChoice,
        std::int32_t playerId,
        std::size_t nameLength
    ) {
        // 1900: year base; 2: constant of the original formula
        const int base = (birthYear + 1900) / (birthMonth + birthDay + 2) * sunSign * profileChoice;
        // 10: heart numbers are the last decimal digit (0..9)
        return (base + (playerId % 10 + static_cast<int>(nameLength))) % 10;
    }

}
