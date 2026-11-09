#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_GAUGE_WIPE_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_GAUGE_WIPE_H

#include "formats/Image.h"

namespace client {

    /// @brief Wipes the curved HP / SP gauge picture to a percent: it keeps only the part on one side of a line through a pivot at its top, turning with the percent.
    /// @details Original: SetGaugePicPercent. Pictures up to 100 pixels wide (HP) turn round x 46, wider ones (SP) round x 58.
    /// @param gauge The full gauge picture.
    /// @param percent Percent to show, 0 to 100.
    /// @param detail An HD picture's pixels per game pixel (the line is the game pixels', each HD pixel taken at its middle).
    /// @return The wiped picture.
    formats::Image wipeGauge(const formats::Image& gauge, int percent, int detail = 1);

    /// @brief The percent a gauge shows for a value: rounded up, at least 1 while the value is above 0, 100 when there is no maximum.
    /// @param value Current value.
    /// @param maximum Maximum value.
    /// @return Percent, 0 to 100.
    int gaugePercent(long long value, long long maximum);

}

#endif
