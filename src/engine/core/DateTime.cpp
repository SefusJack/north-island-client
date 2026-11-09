#include "core/DateTime.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>

namespace core {

    namespace {

        constexpr double kUnixEpochDay = 25569.0; // days from the Delphi TDateTime epoch (1899-12-30) to 1970-01-01
        constexpr double kSecondsPerDay = 86400.0; // 24 * 60 * 60
        constexpr std::int64_t kMsPerDay = 86400000; // 24 * 60 * 60 * 1000

        /// @brief A year, month and day of the civil calendar.
        struct CivilDate {
            std::int64_t year;
            std::int64_t month;
            std::int64_t day;
        };

        /// @brief Converts a day count since 1970-01-01 to a civil date (H. Hinnant's civil_from_days).
        /// @param days Days since 1970-01-01.
        /// @return The year, month and day.
        CivilDate civilDateOf(std::int64_t days) {
            // constants of Hinnant's algorithm: 719468 days from 0000-03-01 to 1970-01-01, 146097 days per 400-year era, 36524/1460/365 per century/4 years/year, 153 per 5 months
            const std::int64_t z = days + 719468;
            const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
            const std::int64_t doe = z - era * 146097;
            const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
            const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
            const std::int64_t mp = (5 * doy + 2) / 153;
            const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
            return {yoe + era * 400 + (month <= 2 ? 1 : 0), month, doy - (153 * mp + 2) / 5 + 1};
        }

    }

    double nowAsDateTime() {
        const std::time_t now = std::time(nullptr);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        std::tm utc{};
#ifdef _WIN32
        gmtime_s(&utc, &now);
#else
        gmtime_r(&now, &utc);
#endif
        const double offset = std::difftime(std::mktime(&local), std::mktime(&utc));
        return kUnixEpochDay + (static_cast<double>(now) + offset) / kSecondsPerDay;
    }

    std::string formatDateTime(double dateTime) {
        double whole = 0.0;
        const double fraction = std::modf(dateTime, &whole);
        auto days = static_cast<std::int64_t>(whole) - static_cast<std::int64_t>(kUnixEpochDay);
        std::int64_t ms = std::llround(std::fabs(fraction) * static_cast<double>(kMsPerDay));
        if (ms >= kMsPerDay) {
            ms -= kMsPerDay;
            ++days;
        }
        const CivilDate date = civilDateOf(days);
        // 1000 ms per second, 3600 s per hour, 60 s per minute; text buffer of 32 chars
        const std::int64_t seconds = ms / 1000;
        char text[32];
        std::snprintf(
            text,
            sizeof text,
            "%04lld/%02lld/%02lld %02lld:%02lld:%02lld",
            static_cast<long long>(date.year),
            static_cast<long long>(date.month),
            static_cast<long long>(date.day),
            static_cast<long long>(seconds / 3600),
            static_cast<long long>(seconds / 60 % 60),
            static_cast<long long>(seconds % 60)
        );
        return text;
    }

    std::string formatDateTimeToStr(double dateTime) {
        double whole = 0.0;
        const double fraction = std::modf(dateTime, &whole);
        std::int64_t ms = std::llround(std::fabs(fraction) * static_cast<double>(kMsPerDay));
        if (ms >= kMsPerDay) {
            ms -= kMsPerDay;
            whole += 1.0;
        }
        std::string text = formatShortDate(whole);
        if (ms != 0) {
            // 1000 ms per second, 3600 s per hour, 60 s per minute, 12-hour clock; text buffer of 32 chars
            const std::int64_t seconds = ms / 1000;
            const std::int64_t hour = seconds / 3600;
            char clock[32];
            std::snprintf(
                clock,
                sizeof clock,
                " %lld:%02lld:%02lld %s",
                static_cast<long long>(hour % 12 == 0 ? 12 : hour % 12),
                static_cast<long long>(seconds / 60 % 60),
                static_cast<long long>(seconds % 60),
                hour < 12 ? "AM" : "PM"
            );
            text += clock;
        }
        return text;
    }

    std::string formatDateTimeAmPm(double dateTime) {
        double whole = 0.0;
        const double fraction = std::modf(dateTime, &whole);
        auto days = static_cast<std::int64_t>(whole) - static_cast<std::int64_t>(kUnixEpochDay);
        std::int64_t ms = std::llround(std::fabs(fraction) * static_cast<double>(kMsPerDay));
        if (ms >= kMsPerDay) {
            ms -= kMsPerDay;
            ++days;
        }
        const CivilDate date = civilDateOf(days);
        // 1000 ms per second, 3600 s per hour, 60 s per minute, 12-hour clock; text buffer of 32 chars
        const std::int64_t seconds = ms / 1000;
        const std::int64_t hour = seconds / 3600;
        char text[32];
        std::snprintf(
            text,
            sizeof text,
            "%02lld/%02lld/%04lld %02lld:%02lld%s",
            static_cast<long long>(date.month),
            static_cast<long long>(date.day),
            static_cast<long long>(date.year),
            static_cast<long long>(hour % 12 == 0 ? 12 : hour % 12),
            static_cast<long long>(seconds / 60 % 60),
            hour < 12 ? "am" : "pm"
        );
        return text;
    }

    std::string formatDateTimeLeading(double dateTime) {
        double whole = 0.0;
        const double fraction = std::modf(dateTime, &whole);
        auto days = static_cast<std::int64_t>(whole) - static_cast<std::int64_t>(kUnixEpochDay);
        std::int64_t ms = std::llround(std::fabs(fraction) * static_cast<double>(kMsPerDay));
        if (ms >= kMsPerDay) {
            ms -= kMsPerDay;
            ++days;
        }
        const CivilDate date = civilDateOf(days);
        // 1000 ms per second, 3600 s per hour, 60 s per minute, 12-hour clock; text buffer of 40 chars
        const std::int64_t seconds = ms / 1000;
        const std::int64_t hour = seconds / 3600;
        char text[40];
        std::snprintf(
            text,
            sizeof text,
            "%lld/%lld/%lld %s%02lld:%02lld",
            static_cast<long long>(date.year),
            static_cast<long long>(date.month),
            static_cast<long long>(date.day),
            hour < 12 ? "AM" : "PM",
            static_cast<long long>(hour % 12 == 0 ? 12 : hour % 12),
            static_cast<long long>(seconds / 60 % 60)
        );
        return text;
    }

    std::string formatShortDate(double dateTime) {
        const CivilDate date = civilDateOf(static_cast<std::int64_t>(std::floor(dateTime)) - static_cast<std::int64_t>(kUnixEpochDay));
        return std::to_string(date.month) + "/" + std::to_string(date.day) + "/" + std::to_string(date.year);
    }

}
