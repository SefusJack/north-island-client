#ifndef NORTH_ISLAND_CORE_DATE_TIME_H
#define NORTH_ISLAND_CORE_DATE_TIME_H

#include <string>

namespace core {

    /// @brief Gives the current local time as a Delphi TDateTime (days since 30 December 1899, fraction = time of day).
    /// @return The date-time value.
    /// @details Original: Delphi's Now.
    double nowAsDateTime();

    /// @brief Formats a date-time as "yyyy/mm/dd hh:mm:ss".
    /// @param dateTime Delphi TDateTime value.
    /// @return The text.
    /// @details Original: FormatDateTime('yyyy"/"mm"/"dd hh:mm:ss').
    std::string formatDateTime(double dateTime);

    /// @brief Formats a date-time with the usual US formats ("M/d/yyyy h:mm:ss AM"); a date with no time of day shows the date only.
    /// @param dateTime Delphi TDateTime value.
    /// @return The text.
    /// @details Original: DateTimeToStr.
    std::string formatDateTimeToStr(double dateTime);

    /// @brief Formats a date-time like "10/18/2026 03:04pm" (the hour is 12 for midnight and noon).
    /// @param dateTime Delphi TDateTime value.
    /// @return The text.
    /// @details Original: FormatDateTime('mm/dd/yyyy hh:nnam/pm').
    std::string formatDateTimeAmPm(double dateTime);

    /// @brief Formats a date-time like "2026/10/18 PM03:04" (12-hour clock, hour padded, 12 for midnight and noon).
    /// @param dateTime Delphi TDateTime value.
    /// @return The text.
    /// @details Original: FormatDateTime('yyyy/m/d ampmhh:mm') with the usual US strings.
    std::string formatDateTimeLeading(double dateTime);

    /// @brief Formats the date part with the usual US short date format ("M/d/yyyy").
    /// @param dateTime Delphi TDateTime value.
    /// @return The text.
    /// @details Original: DateToStr.
    std::string formatShortDate(double dateTime);

}

#endif
